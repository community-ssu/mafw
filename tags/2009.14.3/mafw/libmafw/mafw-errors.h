/*
 * This file is a part of MAFW
 *
 * Copyright (C) 2007, 2008 Nokia, 2009 Corporation, all rights reserved.
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

#ifndef __MAFW_ERRORS_H__
#define __MAFW_ERRORS_H__

/**
 * MafwError:
 * @MAFW_ERROR_PLUGINS_NOT_SUPPORTED:
 * Plug-in loading not supported.
 * @MAFW_ERROR_PLUGIN_LOAD_FAILED:
 * Loading the plug-in failed.
 * @MAFW_ERROR_PLUGIN_INIT_FAILED:
 * Error in plug-in intialization.
 * @MAFW_ERROR_PLUGIN_NAME_CONFLICT:
 * Plug-in with given name already exists.
 * @MAFW_ERROR_PLUGIN_NOT_LOADED:
 * Trying to unload a plug-in which was not loaded.
 *
 * General MAFW error code definitions (plug-in loading).
 */
typedef enum
{
/* Plug-in loading/init related */
	MAFW_ERROR_PLUGINS_NOT_SUPPORTED,
	MAFW_ERROR_PLUGIN_LOAD_FAILED,
	MAFW_ERROR_PLUGIN_INIT_FAILED,
	MAFW_ERROR_PLUGIN_NAME_CONFLICT,
	MAFW_ERROR_PLUGIN_NOT_LOADED,
} MafwError;

/**
 * MafwExtensionError:
 * @MAFW_EXTENSION_ERROR_EXTENSION_NOT_AVAILABLE:
 *   Source or renderer with the specified uuid was not available
 *   or could not be reached.
 * @MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION: 
 *    Tried to invoke an action which was not supported at the moment (e.g. due 
 *    to policy management, media format, missing functionality...)
 * @MAFW_EXTENSION_ERROR_NETWORK_DOWN: 
 *    Bad network conditions or no network at all. Extension cannot connect to the 
 *    service in the network.
 * @MAFW_EXTENSION_ERROR_SERVICE_NOT_RESPONDING: 
 *    Service in the network not responding. Request timed out.
 * @MAFW_EXTENSION_ERROR_EXTENSION_NOT_RESPONDING: 
 *    Extension is busy or not responding. Request timed out.
 * @MAFW_EXTENSION_ERROR_INVALID_PROPERTY: 
 *    Tried to get/set a property which doesn't exist
 * @MAFW_EXTENSION_ERROR_SET_PROPERTY:
 *    Error setting property
 * @MAFW_EXTENSION_ERROR_GET_PROPERTY:
 *    Error getting property
 * @MAFW_EXTENSION_ERROR_ACCESS_DENIED: 
 *    Authorization to extension or service failed.
 * @MAFW_EXTENSION_ERROR_INVALID_PARAMS: 
 *    Action invoked with some invalid parameter, e.g. wrong search filter, 
 *    seek target, etc.
 * @MAFW_EXTENSION_ERROR_OUT_OF_MEMORY: 
 *    System is in low memory state.
 * @MAFW_EXTENSION_ERROR_FAILED: 
 *    Other failure, <code>error->message</code> should explain
 *
 * MAFW error code definitions for common errors for all renderers and sources.
 *
 **/
typedef enum
{
/* Common errors */
  MAFW_EXTENSION_ERROR_EXTENSION_NOT_AVAILABLE,
  MAFW_EXTENSION_ERROR_UNSUPPORTED_OPERATION,
  MAFW_EXTENSION_ERROR_NETWORK_DOWN,
  MAFW_EXTENSION_ERROR_SERVICE_NOT_RESPONDING,
  MAFW_EXTENSION_ERROR_EXTENSION_NOT_RESPONDING,
  MAFW_EXTENSION_ERROR_INVALID_PROPERTY,
  MAFW_EXTENSION_ERROR_SET_PROPERTY,
  MAFW_EXTENSION_ERROR_GET_PROPERTY,
  MAFW_EXTENSION_ERROR_ACCESS_DENIED,
  MAFW_EXTENSION_ERROR_INVALID_PARAMS,
  MAFW_EXTENSION_ERROR_OUT_OF_MEMORY,
  MAFW_EXTENSION_ERROR_FAILED
} MafwExtensionError;

/**
 * MafwPlaylistError:
 * @MAFW_PLAYLIST_ERROR_DATABASE:
 *    Something was wrong with the database access.
 * @MAFW_PLAYLIST_ERROR_INVALID_NAME:
 *    Invalid playlist name.
 * @MAFW_PLAYLIST_ERROR_INVALID_INDEX:
 *    Invalid playlist index (out of bounds).
 * @MAFW_PLAYLIST_ERROR_PLAYLIST_NOT_FOUND:
 *    Couldn't find the playlist with given id or name.
 * @MAFW_PLAYLIST_ERROR_IMPORT_FAILED:
 *    Importing the playlist failed.
 * @MAFW_PLAYLIST_ERROR_INVALID_IMPORT_ID:
 *    Import-ID not found
 *
 * MAFW playlist error code definitions
 *
 **/
typedef enum
{
/* Playlist errors */
  MAFW_PLAYLIST_ERROR_DATABASE,
  MAFW_PLAYLIST_ERROR_INVALID_NAME,
  MAFW_PLAYLIST_ERROR_INVALID_INDEX,
  MAFW_PLAYLIST_ERROR_PLAYLIST_NOT_FOUND,
  MAFW_PLAYLIST_ERROR_IMPORT_FAILED,
  MAFW_PLAYLIST_ERROR_INVALID_IMPORT_ID
} MafwPlaylistError;

/**
 * MafwRendererError:
 * @MAFW_RENDERER_ERROR_NO_MEDIA: 
 *    Tried to play when there is no media (e.g. no playlist)
 * @MAFW_RENDERER_ERROR_URI_NOT_AVAILABLE: 
 *    Renderer couldn't obtain a URI to a media object.
 * @MAFW_RENDERER_ERROR_INVALID_URI: 
 *    The URI of a media object is not valid. The object cannot be played.
 * @MAFW_RENDERER_ERROR_TYPE_NOT_AVAILABLE: 
 *    Renderer couldn't obtain a mime-type to a media object.
 * @MAFW_RENDERER_ERROR_MEDIA_NOT_FOUND:
 *    Renderer could not open a media.
 * @MAFW_RENDERER_ERROR_STREAM_DISCONNECTED:
 *    Renderer could not carry on playing the current media anymore.
 * @MAFW_RENDERER_ERROR_UNSUPPORTED_TYPE: 
 *    The media item mime-type is not supported. Renderer cannot play it.
 * @MAFW_RENDERER_ERROR_DRM:
 *    The media item has drm protection and cannot be played.
 * @MAFW_RENDERER_ERROR_DEVICE_UNAVAILABLE: 
 *    Tried to play media while a system sound was playing.
 * @MAFW_RENDERER_ERROR_CORRUPTED_FILE: 
 *    Error decoding media stream.
 * @MAFW_RENDERER_ERROR_PLAYLIST_PARSING:
 *    Renderer tried to parse a media playlist and it corrupted (or empty).
 * @MAFW_RENDERER_ERROR_CODEC_NOT_FOUND:
 *    Renderer could not find suitable coded for the given media.
 * @MAFW_RENDERER_ERROR_NO_PLAYLIST:
 *    Renderer does not have a playlist.
 * @MAFW_RENDERER_ERROR_INDEX_OUT_OF_BOUNDS:
 *    Attempt to use invalid playlist index.
 * @MAFW_RENDERER_ERROR_CANNOT_PLAY:
 *    Renderer is unable to start playback.
 * @MAFW_RENDERER_ERROR_CANNOT_STOP:
 *    Renderer is unable to stop playback.
 * @MAFW_RENDERER_ERROR_CANNOT_PAUSE:
 *    Renderer is unable to pause playback.
 * @MAFW_RENDERER_ERROR_CANNOT_SET_POSITION:
 *    Renderer is unable to seek to the given position.
 * @MAFW_RENDERER_ERROR_CANNOT_GET_POSITION:
 *    Renderer is unable to retrieve its current playback position.
 * @MAFW_RENDERER_ERROR_CANNOT_GET_STATUS:
 *    Renderer is unable to retrieve its status information.
 * 
 * MAFW renderer specific error code definitions
 *
 **/
typedef enum
{
  MAFW_RENDERER_ERROR_NO_MEDIA,
  MAFW_RENDERER_ERROR_URI_NOT_AVAILABLE,
  MAFW_RENDERER_ERROR_INVALID_URI,
  MAFW_RENDERER_ERROR_MEDIA_NOT_FOUND,
  MAFW_RENDERER_ERROR_STREAM_DISCONNECTED,
  MAFW_RENDERER_ERROR_TYPE_NOT_AVAILABLE,
  MAFW_RENDERER_ERROR_UNSUPPORTED_TYPE,
  MAFW_RENDERER_ERROR_DRM,
  MAFW_RENDERER_ERROR_DEVICE_UNAVAILABLE,
  MAFW_RENDERER_ERROR_CORRUPTED_FILE,
  MAFW_RENDERER_ERROR_PLAYLIST_PARSING,
  MAFW_RENDERER_ERROR_CODEC_NOT_FOUND,
  MAFW_RENDERER_ERROR_NO_PLAYLIST,
  MAFW_RENDERER_ERROR_INDEX_OUT_OF_BOUNDS,
  MAFW_RENDERER_ERROR_CANNOT_PLAY,
  MAFW_RENDERER_ERROR_CANNOT_STOP,
  MAFW_RENDERER_ERROR_CANNOT_PAUSE,
  MAFW_RENDERER_ERROR_CANNOT_SET_POSITION,
  MAFW_RENDERER_ERROR_CANNOT_GET_POSITION,
  MAFW_RENDERER_ERROR_CANNOT_GET_STATUS
/* TODO: gstreamer errors */
/* TODO: dsp errors */
/* TODO: helix errors */
/* TODO: get_metadata related */
/* TODO: policy related */
} MafwRendererError;

/**
 * MafwSourceError:
 * @MAFW_SOURCE_ERROR_INVALID_OBJECT_ID:
 *   Object id syntax not valid.
 * @MAFW_SOURCE_ERROR_OBJECT_ID_NOT_AVAILABLE:
 *   Item with the specified object id was not found.
 * @MAFW_SOURCE_ERROR_INVALID_SEARCH_STRING:
 *   Invalid search filter syntax.
 * @MAFW_SOURCE_ERROR_INVALID_SORT_STRING:
 *   Invalid sort criteria syntax.
 * @MAFW_SOURCE_ERROR_INVALID_BROWSE_ID:
 *   The specified browse id didn't exist.
 * @MAFW_SOURCE_ERROR_PEER:
 *   Error occurred in the peer.
 * @MAFW_SOURCE_ERROR_BROWSE_RESULT_FAILED:
 *   Browse result parsing failed
 * @MAFW_SOURCE_ERROR_GET_METADATA_RESULT_FAILED:
 *   Metadata result parsing failed
 * @MAFW_SOURCE_ERROR_UNSUPPORTED_METADATA_KEY:
 *   Source doesn't support the provided metadata key
 * 
 * MAFW source specific error code definitions
 *
 */
typedef enum
{
/* Source errors */
  MAFW_SOURCE_ERROR_INVALID_OBJECT_ID,
  MAFW_SOURCE_ERROR_OBJECT_ID_NOT_AVAILABLE,
  MAFW_SOURCE_ERROR_INVALID_SEARCH_STRING,
  MAFW_SOURCE_ERROR_INVALID_SORT_STRING,
  MAFW_SOURCE_ERROR_INVALID_BROWSE_ID,  /* Or bundle to invalid_params? */
  MAFW_SOURCE_ERROR_PEER,
  MAFW_SOURCE_ERROR_BROWSE_RESULT_FAILED,
  MAFW_SOURCE_ERROR_GET_METADATA_RESULT_FAILED,
  MAFW_SOURCE_ERROR_UNSUPPORTED_METADATA_KEY,
  MAFW_SOURCE_ERROR_DESTROY_OBJECT_FAILED,
  MAFW_SOURCE_ERROR_PLAYLIST_PARSING_FAILED
} MafwSourceError;

/**
 * MAFW_ERROR:
 *
 * Gets a quark for general mafw error
 */
#define MAFW_ERROR g_quark_from_static_string("com.nokia.mafw.error")

/**
 * MAFW_EXTENSION_ERROR:
 *
 * Gets a quark for common renderer/source errors
 */
#define MAFW_EXTENSION_ERROR g_quark_from_static_string("com.nokia.mafw.error.extension")

/**
 * MAFW_RENDERER_ERROR:
 *
 * Gets a quark for renderer errors
 */
#define MAFW_RENDERER_ERROR g_quark_from_static_string("com.nokia.mafw.error.renderer")

/**
 * MAFW_SOURCE_ERROR:
 *
 * Gets a quark for source errors
 */
#define MAFW_SOURCE_ERROR g_quark_from_static_string("com.nokia.mafw.error.source")

/**
 * MAFW_PLAYLIST_ERROR:
 *
 * Gets a quark for playlists errors
 */
#define MAFW_PLAYLIST_ERROR g_quark_from_static_string("com.nokia.mafw.error.playlist")

#endif

