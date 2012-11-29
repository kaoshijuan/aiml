
METASOURCES = AUTO

SUBDIRS = 
AM_CXXFLAGS = -Wall $(WERROR_CXXFLAG) -utf8 -pedantic `xml2-config --cflags`

noinst_PROGRAMS = server_app
server_app_SOURCES = main.cpp  tlib_log.cpp

server_app_LDADD = $(top_builddir)/src/libaiml.a $(PCRECPP_LIBS) -L./ -lstd_utils -lz -lxml2
server_app_LDFLAGS = $(XML2_LIBS)

