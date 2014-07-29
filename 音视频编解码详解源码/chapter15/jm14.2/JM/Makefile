###
###     Makefile for JM H.264/AVC encoder/decoder
###
###             generated for UNIX/LINUX environments
###             by Limin Wang(lance.lmwang@gmail.com) 
###

SUBDIRS := lencod ldecod rtpdump rtp_loss

### include debug information: 1=yes, 0=no
DBG?= 0
### include M32 optimization : 1=yes, 0=no
M32?= 0

export DBG
export M32

.PHONY: default all distclean clean tags depend $(SUBDIRS)

default: all

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean depend:
	for i in $(SUBDIRS); do make -C $$i $@; done

tags:
	@echo "update tag table at top directory"
	ctags -R .
	for i in $(SUBDIRS); do make -C $$i $@; done

distclean: clean
	rm -f tags
	for i in $(SUBDIRS); do make -C $$i $@; done

