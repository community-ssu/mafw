<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:confml="http://www.s60.com/xml/confml/2">
	
<xsl:output method="xml" indent="yes" encoding="UTF-8"/>

<!-- Start processing with configuration node -->
<xsl:template match="/confml:configuration">
    <xsl:copy>
	    <!--<xsl:apply-templates select="confml:data/confml:mafw-iradio-source-bookmarks"/>-->
	    <xsl:element name="data" namespace="http://www.s60.com/xml/confml/2">
		    <xsl:copy-of select="confml:data/confml:mafw-iradio-source-bookmarks"/>
	    </xsl:element>
    </xsl:copy>
</xsl:template>
</xsl:stylesheet>
