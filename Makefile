LDFLAGS+= -lmraa -lutil -lboost_iostreams -lboost_system -lboost_filesystem

all: accel

accel: SFE_LSM9DS0.o main.o
	g++ SFE_LSM9DS0.o main.o -o accel $(LDFLAGS)

9dof.o: SFE_LSM9DS0.cpp
	g++ -c SFE_LSM9DS0.cpp -o SFE_LSM9DS0.o

main.o: main.cpp
	g++ -c main.cpp -o main.o

clean:
	rm *.o accel

#install: install-files
#    groupadd -f --system gpio
#    chgrp gpio $(DESTDIR)/bin/gpio-admin
#    chmod u=rwxs,g=rx,o= $(DESTDIR)/bin/gpio-admin
