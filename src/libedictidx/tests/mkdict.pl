#!/usr/bin/perl

$dict = $ARGV[0] // die ("Usage:\n\tmkdict.pl dictfile [entry_num]\n");
$n = $ARGV[1] // 100;
$maxword = 10;
$maxwords = 5;
$maxtypes = 1;
$maxdefs = 10;

@letters = ('a'..'z', 'A'..'Z');

open(DICT, ">", $dict) or die $!;

sub randletter {
	$letters[int(0.5+rand($#letters))];
}

sub randword {
	my $len = int(0.5+rand($maxword));
	join("", map(randletter, (1..$len)));
}

sub randtrans {
	return "[" . randword . "]" if (rand(10) > 1);
	return "[" . randword if (rand(2) > 1);
	return randword . "]" if (rand(2) > 1);
}

sub randphrase {
	my $len = int(0.5+rand($maxwords));
	join(" ", map(randword, (1..$len)));
}

sub randtype {
	return "(" . randword . ")" if (rand(10) > 1);
	return "(" . randword if (rand(2) > 1);
	return randword . ")" if (rand(2) > 1);
}

sub randtypes {
	my $len = int(0.5+rand($maxtypes));
	join(" ", map(randtype, (1..$len)));
}

sub randdef {
	join(" ", randtypes, randphrase) =~ /^ *(.*?) *$/;
}

sub randdefs {
	my $len = int(0.5+rand($maxdefs));
	"/" . join("/", map(randdef, (1..$len))) . "/";
}

sub randentry {
	my ($format) = @_;

	@arglist = (randword, randtrans, randdefs);
	printf DICT ($format, @arglist);
}

foreach $i (1..$n) {
	randentry("%s %s %s\n");
	print("${i} words...\n") if ($i % 1000 == 0);
}

print("DONE\n");
