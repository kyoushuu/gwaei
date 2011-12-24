#!/bin/sh

testdict="test.edict"

case $1 in
    all)
	echo "Running all tests"
	checkkanji=y;
	checkkana=y;
	checkenglish=y;
	;;

    kanji)
	echo "Running kanji tests"
	checkkanji=y;
	;;

    kana)
	echo "Running kana tests"
	checkkana=y;
	;;

    english)
	echo "Running english tests"
	checkenglish=y;
	;;

    clean)
	rm -f ${testdict} *.idx *.dump
	exit 0
	;;

    *)
	echo "Usage:"
	echo "        runtests.sh (all | kanji | kana | english | clean) [dictfile]"
	exit 1
	;;
esac

function die {
    echo "!!! Test failed !!!"
    exit 1
}

if [ -n "$2" ] ; then
    dict=$2;
else
    dict=$testdict

    echo "> generating random dictionary..."
    ./mkdict.pl $dict 10000 || die
fi

echo "> parsing kanji dictionary..."
./parser $dict 1 > ${dict}.kanji.dump || die
echo "> parsing kana dictionary..."
./parser $dict 2 > ${dict}.kana.dump || die
echo "> parsing english dictionary..."
./parser $dict 3 > ${dict}.english.dump || die

echo "> building kanji index..."
./build $dict ${dict}.kanji.idx 1 || die
echo "> building kana index..."
./build $dict ${dict}.kana.idx 2 || die
echo "> building english index..."
./build $dict ${dict}.english.idx 3 || die

echo "> verifying kanji index..."
./verify $dict ${dict}.kanji.idx 1 || die
echo "> verifying kana index..."
./verify $dict ${dict}.kana.idx 2 || die
echo "> verifying english index..."
./verify $dict ${dict}.english.idx 3 || die

if [ "$checkkanji" = y ] ; then
    echo "> checking kanji index..."
    ./checkindex.pl $dict ${dict}.kanji.idx 1 || die
fi

if [ "$checkkana" = y ] ; then
    echo "> checking kana index..."
    ./checkindex.pl $dict ${dict}.kana.idx 2 || die
fi

if [ "$checkenglish" = y ] ; then
    echo "> checking english index..."
    ./checkindex.pl $dict ${dict}.english.idx 3 || die
fi
