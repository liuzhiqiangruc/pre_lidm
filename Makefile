UTILDIR  = util
REGRDIR  = regr
TMDIR    = tm
SRC      = src
INSTDIR  = .



all: libs app

libs:
	cd $(UTILDIR); $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(REGRDIR); $(MAKE) clean; $(MAKE); $(MAKE) install
	cd $(TMDIR);   $(MAKE) clean; $(MAKE); $(MAKE) install

app :
	cd $(SRC); $(MAKE) clean ; $(MAKE); $(MAKE) install


# --------------
# install
# --------------
install:
	cp -r lib output
	cp -r inc output

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------

.PHONY : localclean clean

localclean:
	rm -rf output
	rm -rf inc
	rm -rf lib

clean:
	$(MAKE) localclean
	cd $(UTILDIR);  $(MAKE) clean
	cd $(REGRDIR);  $(MAKE) clean
	cd $(SRC);      $(MAKE) clean
	cd $(TMDIR);    $(MAKE) clean
