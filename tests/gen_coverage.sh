#! /bin/sh
set -e

rm -f coverage.src coverage

for src in ${SOURCES}; do
	case "$src" in
		*.c)
			obj=`echo $src | sed 's|\.c|.o|'`
			if test -f "${builddir}/.libs/libmongo_client_la-$obj"; then
				objdir=${builddir}/.libs
			else
				objdir=${builddir}
			fi;
			gcov ${srcdir}/$src -o $objdir/libmongo_client_la-$obj >>coverage.src
			;;
	esac
done

for src in ${SOURCES}; do
	case "$src" in
		*.c)
			grep -A3 "File '${srcdir}/$src'" coverage.src >>coverage
			;;
	esac
done

rm -f coverage.src

c=`(echo "scale=2"; echo -n "("; echo -n $(grep "Lines executed" coverage | cut -d: -f2 | cut -d "%" -f 1) | sed -e "s, , + ,g"; echo ") / " $(grep -c "Lines executed" coverage)) | bc -q`; \
echo "Overall coverage: $c%" >>coverage
