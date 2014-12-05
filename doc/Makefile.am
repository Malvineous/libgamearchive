man_MANS = gamearch.1
man_MANS += gamecomp.1

EXTRA_DIST = gamearch.xml
EXTRA_DIST += gamecomp.xml
EXTRA_DIST += camoto.xsl

# Also distribute the converted man pages so users don't need DocBook installed
EXTRA_DIST += $(man_MANS)

HTML_MAN = gamearch.html
HTML_MAN += gamecomp.html

.PHONY: html

html: $(HTML_MAN)

MAINTAINERCLEANFILES = $(man_MANS) $(HTML_MAN)

%.1: %.xml camoto.xsl
	xmlto --skip-validation man -m $(<D)/camoto.xsl $<

%.html: %.xml camoto.xsl
	xmlto --skip-validation xhtml-nochunks -m $(<D)/camoto.xsl $<
