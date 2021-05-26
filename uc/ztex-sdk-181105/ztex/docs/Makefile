.PHONY: docs java

all: java c

java:
	javadoc ztex -classpath ../java:../usb4java -d java

c:
	doxygen 

java2:
	doxygen Doxyfile.java

distclean: 
	rm -fr java java2 c
