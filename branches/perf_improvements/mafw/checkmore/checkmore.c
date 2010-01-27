/*
 * This file is a part of MAFW
 *
 * Copyright (C) 2007, 2008, 2009 Nokia Corporation, all rights reserved.
 *
 * Contact: Visa Smolander <visa.smolander@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/* For strsignal() */
#define _GNU_SOURCE

/* Include files */
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>

#include <glib.h>
#include <glib-object.h>
#include <check.h>

#include <libmafw/mafw-log.h>

#include "checkmore.h"

/* Standard definitions */
/* Tell g_message() and friends who we are. */
#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN		"checkmore"

/*
 * How to start the dbus-daemon.
 * This can be tricky, so let's deal with it seaprately.
 * These macros are substituted in execv() calls.  START_SESSION_BUS includes
 * the executable's name because it is evaluated in the first argument of the
 * function call.
 */
#if defined(__ARMEL__) && !__GNUC_PREREQ(4, 2)
/*
 * This is for d****o + qemu-arm-cvs-m + dbus 1.2.  This version of qemu
 * has issues with that version of dbus, so we use the recommended emulator
 * to run the daemon in this case.  Unfortunately if dbus is compiled with
 * the newer toolchain this doesn't help, qemu segfaults.
 */
# define START_SYSTEM_BUS	"qemu-arm-0.8.2-sb2", \
				"/usr/bin/dbus-daemon",		"--system"
# define START_SESSION_BUS	"qemu-arm-0.8.2-sb2", "qemu", \
				"/usr/bin/dbus-daemon",		"--session"
#else
# define START_SYSTEM_BUS	"dbus-daemon",			"--system"
# define START_SESSION_BUS	"dbus-daemon", "dbus-daemon",	"--session"
#endif

/* Private variables */
/*
 * $Msg_filters:	Array of #GPatternSpec:s telling what log messages
 *			to make less important.
 * $Mainloop:           Main loop used by checkmore_{spin,stop}_loop().
 */
static GPtrArray *Msg_filters		= NULL;
static GMainLoop *Mainloop      	= NULL;

/*
 * $Daemon:		The PID of the program started by checkmore_start().
 *			-1 if no such program is running.
 * $Daemon_died:	TRUE if the checkmore_start()ed program terminated,
 *			but not with $Daemon_ok_code.
 * $Daemon_ok_code:	The accepted exit code of the checkmore_start()ed
 *			program.  -1 means the program may not exit().
 */
static pid_t Daemon			= -1;
static gboolean Daemon_died		= FALSE;
static gint Daemon_ok_code		= -1;

/*
 * $Me:			That's me, the PID of the process governing the
 *			test cases.  Used to prevent kill_dbus() from
 *			doing its job on behalf of the children.
 * $*_bus_pid:		PID of dbus-daemon we started; killed by the parent
 *			process on normal exit.
 */
static pid_t Me			= -1;
static pid_t System_bus_pid	= -1;
static pid_t Session_bus_pid	= -1;

/* Global variable definitions */
/* These variables are used to communicate expectations between
 * the expect_() macros and the overridden g_*() functions. */
gboolean CheckMore_expect_assert   = FALSE;
guint    CheckMore_expect_fallback = 0;

/* Program code */
/* Private functions */
/* SIGCHLD handler to tell what happened to $Daemon. */
static void daemon_died(int unused)
{
	int status;

	/* Let `check' reap everyone else.
	 * NOTE that we must not fail() or exit() or anything
	 * otherwise the test gets very confused. */
	if (Daemon < 0 || waitpid(Daemon, &status, WNOHANG) != Daemon)
		return;
	Daemon_died = checkmore_child_died(status, "daemon", Daemon_ok_code);
	Daemon = -1;
}

/* Tells whether $path is a libtool-wrapper of a real program. */
static gboolean is_libtool_victim(const char *path)
{
	int hfd;
	char shebang[2];

	/* Return if it's a shell script. */
	memset(shebang, 0, sizeof(shebang));
	if ((hfd = open(path, O_RDONLY)) < 0)
	{
		fail("%s: %s", strerror(errno));
		exit(-1);
	}
	read(hfd, shebang, sizeof(shebang));
	close(hfd);

	return shebang[0] == '#' && shebang[1] == '!';
}

/* Set $LD_PRELOAD to what libsb would set it if it were active
 * and were to start a target binary. */
static void preload_hack(void)
{
	const char *sbox_preload, *preload;

	if (!(sbox_preload = getenv("SBOX_PRELOAD"))) {
		g_warning("$SBOX_PRELOAD doesn't exist -- i smell trouble");
		return;
	}

	/*
	 * $SBOX_PRELOAD is a pair of library lists: "<host>,<target>",
	 * where <host> is set to $LD_PRELOAD if starting a host binary
	 * and <target> if (surprise) starting a target binary.
	 * Since we're interested in target binaries we need <target>.
	 * If there's no comma suppose <host> is empty.
	 */
	if ((preload = strchr(sbox_preload, ',')) != NULL)
		preload++;
	else
		preload = sbox_preload;

	setenv("LD_PRELOAD", preload, 1);
}

/*
 * Starts a dbus-daemon of $bustype and sets its address in the $env
 * environment variable.  Returns the pid of the daemon or -1 if we
 * didn't start it after all.
 */
static pid_t start_dbus(gboolean is_system)
{
	pid_t pid;
	int comm[2];
	FILE *commst;
	char addr[128], *nl;

	/* Start the dbus-daemon, communicate the bus address through a pipe. */
	if (pipe(comm) < 0)
		g_error("pipe: %m");
	if (!(pid = fork())) {
		const char *bustype;

		/* Let dbus-daemon write its address to STDOUT_FILENO.
		 * If we send a single newline our parent will know
		 * something is wrong. */
		close(comm[0]);
		if (comm[1] != STDOUT_FILENO) {
			if (dup2(comm[1], STDOUT_FILENO) < 0) {
				write(comm[1], "\n", 1);
				g_error("dup2: %m");
			}
			close(comm[1]);
		}

		bustype = is_system ? "--system" : "--session";
		g_warning("Starting dbus-daemon %s", bustype);
		if (is_system) {
			/*
			 * The system bus wants to be root to change user.
			 * The fakeroot script in scratchbox only sets
			 * $SBOX_PRELOAD, not $LD_PRELOAD.  It is libsb
			 * that copies the former environment variable
			 * to the latter, turning the override effective.
			 * Unfortunately if we are not overridden by libsb
			 * (and turns out not to be) this does not happen,
			 * rendering fakeroot ineffective and causing the
			 * system bus startup to fail.
			 */
			preload_hack();
			execlp("fakeroot", "fakeroot", START_SYSTEM_BUS,
			       "--print-address=1", "--print-pid=1", NULL);
		} else
			execlp(START_SESSION_BUS,
			       "--print-address=1", "--print-pid=1", NULL);
		write(STDOUT_FILENO, "\n", 1);
		g_error("dbus_daemon %s: %m", bustype);
	} /* if */

	close(comm[1]);
	commst = fdopen(comm[0], "r");

	/* Read and parse the bus address, rip off the trailing newline. */
	if (!fgets(addr, sizeof(addr), commst)) {
		/* dbus-daemon exited; assume it's already running. */
		pid = -1;
		goto out;
	}
	if (!(nl = strchr(addr, '\n')))
		g_error("unterminated line from dbus-daemon: %s", addr);
	*nl = '\0';
	if (!addr[0])
		g_error("dbus-daemon didn't start up");

	/* Don't believe fork() because the PID it returned is fakeroot's
	 * in case of the system bus, which we'd better not kill. */
	if (fscanf(commst, "%u", &pid) != 1)
		g_error("garbage instead of PID from dbus-daemon");

	/* Populate the bus address. */
	setenv(is_system
	       ? "DBUS_SYSTEM_BUS_ADDRESS"
	       : "DBUS_SESSION_BUS_ADDRESS",
	       addr, TRUE);
out:	fclose(commst);
	return pid;
}

/* atexit() callback to kill the dbus-daemon:s we started.
 * This is only supposed to run in the parent process. */
static void kill_dbus(void)
{
	if (getpid() != Me)
		/* We're not the parent, don't mess with the daemons. */
		return;
	if (System_bus_pid > 0)
		kill(System_bus_pid,  SIGTERM);
	if (Session_bus_pid > 0)
		kill(Session_bus_pid, SIGTERM);
}

/* Function overrides */
/*
 * This function is called by GLib when the an assertion evaluated
 * by g_assert() fails.  If $CheckMore_expect_assert is FALSE it is
 * should behave just like the original function, otherwise it omits
 * the error message.
 */
void g_assert_warning(const char *domain,
		      const char *file, int line,
		      const char *fun, const char *expr)
{
	if (!CheckMore_expect_assert)
		g_log(domain, G_LOG_LEVEL_ERROR,
		      "file %s: line %d (%s): assertion failed: %s",
		      file, line, fun, expr);
	abort();
}

/* This is called by GLib >= 2.16 for assertions.  Behaviour is equivalent to
 * the above. */
void g_assertion_message_expr(const char *domain,
			      const char *file, int line,
			      const char *func, const char *exp)
{
	if (!CheckMore_expect_assert)
		g_log(domain, G_LOG_LEVEL_ERROR,
		      "file %s: line %d (%s): assertion failed: %s",
		      file, line, func, exp);
	abort();
}

/*
 * This function is called by GLib when g_return_if_fail() detects an
 * error, suppressing its message as long as $CheckMore_expect_fallback.
 * After that it should behave like the original function.
 */
void g_return_if_fail_warning(const char *domain,
			      const char *fun,
			      const char *expr)
{
	if (CheckMore_expect_fallback > 0) {
		/* NOTE If this counter overflows expect_ignore() and
		 * expect_fallback() will complain for no reason. */
		CheckMore_expect_fallback++;
		return;
	}

	g_log(domain, G_LOG_LEVEL_CRITICAL, "%s: assertion `%s' failed",
	      fun, expr);
}

/* Override vanilla g_log() to take $Msg_filter into account.
 * Cannot use #GLogFunc:s because mafw-log may (and should) be active. */
void g_log(const gchar *domain, GLogLevelFlags level, const gchar *fmt, ...)
{
	guint i;
	va_list args;

	/* Change the level of the message to G_LOG_LEVEL_INFO if it matches
	 * any of $Msg_filters.  It only makes sense if the level is above. */
	if (Msg_filters && (level & (G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING
		     | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR))) {
		for (i = 0; i < Msg_filters->len; i++)
			if (g_pattern_match_string(Msg_filters->pdata[i], fmt)){
				level &= ~G_LOG_LEVEL_MASK;
				level |=  G_LOG_LEVEL_INFO;
				break;
			}
	}

	va_start(args, fmt);
	g_logv(domain, level, fmt, args);
	va_end(args);
}

#ifdef __ARMEL__
/* qemu is angry when it sees GLib trying to poll(NULL, 0, ...). */
int poll(struct pollfd *pfd, nfds_t npfd, int timeout)
{
	if (!pfd && !npfd) {
		/* Empty set, emulate with select(2). */
		if (timeout >= 0) {
			struct timeval tv;

			tv.tv_sec  =  timeout / 1000;
			tv.tv_usec = (timeout % 1000) * 1000;
			return select(0, NULL, NULL, NULL, &tv);
		} else
			return select(0, NULL, NULL, NULL, NULL);
	} else {
		/* Emulate with ppoll(2). */
		if (timeout >= 0) {
			struct timespec ts;

			ts.tv_sec  =  timeout / 1000;
			ts.tv_nsec = (timeout % 1000) * 1000000;
			return ppoll(pfd, npfd, &ts, NULL);
		} else
			return ppoll(pfd, npfd, NULL, NULL);
	}
}
#endif /* __ARMEL__ */

/* Interface functions */
/** Sets $environ[$key] to the absolute path to $fname. */
void checkmore_set_absolute_env(const gchar *key, const gchar *fname)
{
	gchar *cwd, *path;

	cwd = g_get_current_dir();
	path = g_strjoin(cwd, "/", fname, NULL);
	if (!g_setenv(key, path, 0))
		g_error("g_setenv(): %m");
	g_free(path);
	g_free(cwd);
}

/**
 * If @fname is not %NULL redirects %stdout and %stderr there, so all
 * messages eg. from g_log() will end up in that file.  Otherwise
 * NOP. */
void checkmore_redirect(char const *fname)
{
	int pipefd[2] = { 1, 2 };

	if (fname && !freopen(fname, "a", stderr))
		g_error("%s: %m", fname);
	if (fname && !freopen(fname, "a", stdout))
		g_error("%s: %m", fname);
}

/**
 * Make messages matching $pat, a g_pattern expression appear as if
 * they were informational (#g_info()).  By configuring mafw-log you
 * can get rid of those messages from the program output.
 *
 * NOTE @pat is matched against the message format string, which may
 * or may not be what you need.
 */
void checkmore_ignore(gchar const *pat)
{
	if (pat != NULL) {
		if (!Msg_filters)
			Msg_filters = g_ptr_array_new();
		g_ptr_array_add(Msg_filters, g_pattern_spec_new(pat));
	} else if (Msg_filters)
	{
		guint i;

		/* Clear all ignores. */
		for (i = 0; i < Msg_filters->len; i++)
			g_pattern_spec_free(Msg_filters->pdata[i]);
		g_ptr_array_set_size(Msg_filters, 0);
	}
}

/**
 * Creates and returns a test case for @suite with @name and adds @fun.
 * Useful when you are defining test cases with a single test, so you
 * can disable it more easily.
 */
TCase *checkmore_add_tcase(Suite *suite, char const *name, TFun fun)
{
	TCase *tcase;

	tcase = tcase_create(name);
	suite_add_tcase(suite, tcase);
	tcase_add_test(tcase, fun);

	return tcase;
}

/** Adds @fun expected to be abort()ed by g_assert() to @tc. */
void checkmore_add_aborting_test(TCase *tc, TFun fun)
{
	tcase_add_test_raise_signal(tc, fun, SIGABRT);
}

/**
 * Runs @runner and returns whether any tests failed.  Doesn't run tests
 * in separate processes if @nofork.
 */
int checkmore_run(SRunner *runner, gboolean nofork)
{
	int nfailed;

	g_type_init();
	mafw_log_init(NULL);

	/* In certain cases you may require or simply prefer nonforking. */
	if (nofork)
		srunner_set_fork_status(runner, CK_NOFORK);

#ifdef __ARMEL__
	/* qemu doesn't like uncaught signals. */
	signal(SIGABRT, SIG_IGN);
#endif

	/*
	 * On timeout `check' kill()s the entire process group sometimes,
	 * including itself, and more problematically, including the parent,
	 * which is rather annoying when you try to run tests from your
	 * editor.  To prevent that get a new process group now.  It is not
	 * a problem if the test is run from a shell because it sets a new
	 * pgroup unconditionally.
	 */
	setpgid(0, 0);

	/* srunner_free() frees the Suite and all TCase:s as well.
	 * The $CK_VERBOSITY environment variable may be used to
	 * control amount of output (defaults to CK_NORMAL). */
	srunner_run_all(runner, CK_ENV);
	nfailed = srunner_ntests_failed(runner);
	srunner_free(runner);

	/* Assuming that nothing else is run after this function, let's unref
	 * the internal mainloop. */
	if (Mainloop) {
		g_main_loop_unref(Mainloop);
		Mainloop = NULL;
	}

	return nfailed != 0;
}

/**
 * Use this function if you intend to connect to the session or system D-BUS.
 * dbusd rejects us if we're run under fakeroot, because libdbus believes we
 * are uid=0 and says it to dbusd, which knows the truth because it gets it
 * straight from the kernel, then rejects us as liar freaks.
 */
void checkmore_unfakeroot(void)
{
	FILE *st;
	char line[64];

	/* Are we in the hands of fakeroot?
	 * (No-one can be insane enough to run us as *real* root.) */
	if (getuid() != 0)
		return;

	/* Figure out our real identity from /proc (which fortunately
	 * is not faked) and switch back, so getuid() won't mischief us. */
	st = fopen("/proc/self/status", "r");
	while (fgets(line, sizeof(line), st)) {
		uid_t uid, euid;

		if (sscanf(line, "Uid: %u %u", &uid, &euid) == 2) {
			if (setreuid(uid, euid) == -1)
				g_warning("Error calling setreuid: %m");
			break;
		}
	}
	fclose(st);
}

/**
 * Makes sure you are not running without system and session D-BUS:es.
 * If there's no such bus it starts a new dbus-daemon to be killed when
 * the test program exits.  It's intended to be called from main().
 * As a side effect after this function returns you won't feel as root.
 */
void checkmore_wants_dbus(void)
{
#if defined(__ARMEL__) && __GNUC_PREREQ(4, 2)
	/* With this toolchain dbus is unusable: qemu-arm-0.8.2-sb2 segfaults,
	 * while qemu-arm-cvs-m is buggy (recvfrom()).  Let's keep peace. */
	g_warning("Possible environmental issues, test skipped.");
	exit(0);
#endif

	/* It is necessary not to connect to the dbus daemons telling
	 * them we're root, because they will refuse the connection. */
	checkmore_unfakeroot();

	Me = getpid();
	atexit(kill_dbus);
	if (!getenv("DBUS_SYSTEM_BUS_ADDRESS")
	    && access("/var/run/dbus/pid", F_OK) < 0)
		/* The system bus won't start anyway if the pidfile exitst. */
		System_bus_pid = start_dbus(TRUE);
	if (!getenv("DBUS_SESSION_BUS_ADDRESS"))
		Session_bus_pid = start_dbus(FALSE);

	/* Give them some time to wake up. */
	g_usleep(500000);
}

/**
 * Starts @path in a separate process and monitor it.  If the program
 * terminates before you checkmore_stop() it then unless it exit()ed
 * with @ok_code (provided it's not negative) it will be reported as
 * an error and checkmore_stop() will fail().  @args is either %NULL
 * or the regular %NULL-terminated string array you'd pass to execve()
 * (including argv[0]).
 */
void checkmore_start(const gchar *path, gint ok_code, const gchar *const *args)
{
	g_assert(Daemon < 0);
	Daemon_died = FALSE;
	Daemon_ok_code = ok_code;

	signal(SIGCHLD, daemon_died);
	if (!(Daemon = fork())) {
		const char *argv[2];

		if (!args) {
			if (!(argv[0] = strrchr(path, '/')))
				argv[0] = path;
			else
				argv[0]++;
			argv[1] = NULL;
			args = argv;
		}

#if __GNUC_PREREQ(4, 2)
		/*
		 * SBOX hack: don't drop LD_PRELOAD, otherwise relinking a
		 * potential libtool "binary" fails, because sb_alien is not
		 * adding the necessary -L and -rpath flags to the linker
		 * command line.  This is only prevalent under the new
		 * toolchain and only if the program to be executed has been
		 * victimized by libtool.
		 */
		if (is_libtool_victim(path))
			preload_hack();
#endif /* gcc >= 4.2 */

		/* Somebody tell me why can't they fix
		 * the type of $argv in exec*()? */
		g_info("Starting %s", path);
		execv(path, (char **)args);
		fail("execv(%s): %s", path, strerror(errno));
	}
}

/** Terminates $Daemon if it's still running.  Otherwise fail()s if it died. */
void checkmore_stop(void)
{
	/* From daemon_died we couldn't fail() but now we can. */
	fail_if(Daemon_died);
	if (Daemon > 0) {
		signal(SIGCHLD, SIG_DFL);
		kill(Daemon, SIGTERM);
		waitpid(Daemon, NULL, 0);
		Daemon = -1;
	}
}

/**
 * Examines @status, the output argument of wait() and prints @who:s fate
 * if it didn't exit() with @ok_code (provided it's not negative).
 */
gboolean checkmore_child_died(int status, const gchar *who, int ok_code)
{
	if (WIFSIGNALED(status))
		g_warning("%s killed by %s", who, strsignal(WTERMSIG(status)));
	else if (!WIFEXITED(status))
		g_warning("%s died nasty death", who);
	else if (ok_code < 0 || WEXITSTATUS(status) != ok_code)
		g_warning("%s exited with unexpected code %u",
			  who, WEXITSTATUS(status));
	else
		return FALSE;
	return TRUE;
}

/**
 * Spins checkmore's internal GMainLoop for @time milliseconds or infinitely if
 * -1 is given.
 */
void checkmore_spin_loop(gint time)
{
	if (!Mainloop)
		Mainloop = g_main_loop_new(NULL, FALSE);
	if (time != -1)
		g_timeout_add(time, (GSourceFunc)g_main_loop_quit, Mainloop);
	g_main_loop_run(Mainloop);
}

/**
 * Stops checkmore's GMainLoop.
 */
void checkmore_stop_loop(void)
{
	if (Mainloop)
		g_main_loop_quit(Mainloop);
}

/* vi: set noexpandtab ts=8 sw=8 cino=t0,(0: */
