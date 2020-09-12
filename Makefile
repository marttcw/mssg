# mssg 

define \n

endef

define SRC
src/parser.c
src/log.c
src/templates.c
src/minify.c
src/files.c
src/config.c
src/copy.c
src/vars.c
endef

SRC := $(strip ${SRC})

NAME = mssg
SRC_MAIN = src/main.c
SRC_TEST = src/test.c
CC = ccache cc
VERSION = PRE-ALPHA-0.1
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LINKS = 
INCLUDES = -Isrc
PKG = 
PKG_CFG =
CFLAGS = -std=c99 -pedantic-errors -pedantic -Wall -Wextra -Wpointer-arith -Wstrict-prototypes -fomit-frame-pointer -ffast-math
CFLAGS_RELEASE = -flto -O3
CFLAGS_DEBUG = -g
OTHER_FILES = LICENSE Makefile README.md

${NAME}: ${SRC_MAIN} ${SRC}
	@echo make release build
	@${CC} -o ${NAME} ${LINKS} ${INCLUDES} ${CFLAGS} ${PKG_CFG} ${CFLAGS_RELEASE} ${SRC_MAIN} ${SRC}

debug:
	@echo make debug build
	@${CC} -o ${NAME} ${LINKS} ${INCLUDES} ${CFLAGS} ${PKG_CFG} ${CFLAGS_DEBUG} ${SRC_MAIN} ${SRC}

test:
	@echo make test build
	@${CC} -o unit_test ${LINKS} ${INCLUDES} ${CFLAGS} ${PKG_CFG} ${CFLAGS_DEBUG} ${SRC_TEST}
#@./unit_test
#@rm unit_test

run: ${NAME}
	@./${NAME}

clean:
	@echo cleaning
	@rm -f ${NAME} ${NAME}-${VERSION}.tar.gz unit_test

dist: clean
	@echo creating dist tarball
	@mkdir -p ${NAME}-${VERSION}
	@cp -R ${OTHER_FILES} ${SRC} ${NAME}.1 ${NAME}-${VERSION}
	@tar -cf - "${NAME}-${VERSION}" | \
		gzip -c > "${NAME}-${VERSION}.tar.gz"
	@rm -rf "${NAME}-${VERSION}"

install: ${NAME}
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin/${NAME}
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f ${NAME} ${DESTDIR}${PREFIX}/bin/.
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < ${NAME}.1 > ${DESTDIR}${MANPREFIX}/man1/${NAME}.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin/${NAME}
	@rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

help:
	@echo "make {|debug|test|run|clean|dist|install|uninstall|help}"

.PHONY: debug test run clean dist install uninstall help

