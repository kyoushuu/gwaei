#!/usr/bin/perl

sub usage {
	die ("Usage:\n\tcheckindex.pl dictfile indexfile keytype\n");
}

$dict = $ARGV[0] // usage;
$idx = $ARGV[1] // usage;
$keytype = $ARGV[2] // usage;

sub kanjikey {
	shift =~ /^([^ ]+)/;
}

sub kanakey {
	shift =~ /\[([^\]]+)\]/;
}

sub englishkey {
	my @k = (shift =~ /\/([^\/]+)/g);
	@k = map { s/\G(\([^)]*\) *)//g; $_;} @k;
	@k = map { s/^(\([^)]*)$//; $_ } @k;
}

%keymap = (1 => \&kanjikey, 2 => \&kanakey, 3 => \&englishkey);

$keyfun = $keymap{$keytype} // die "Unknown key type ${keytype}\n";

open(DICT, "<", $dict) or die $!;

$nentries = 0;
$nkeys = 0;
$nfound = 0;
$nmissing = 0;
$ndupes = 0;
$nextras = 0;

while ($entry = <DICT>) {
	chomp($entry);
	@keys = $keyfun->($entry);
KEYLOOP:
	foreach $key (@keys) {
		$key || next KEYLOOP;
		$found = 0;
		$safekey = $key;
		$safekey =~ s/"/\\"/g;
		open(RESULTS, "-|", "./query ${dict} ${idx} \"${safekey}\"");
		while ($result = <RESULTS>) {
			chomp($result);
			if ($result eq $entry) {
				unless ($found) {
					print("+")
					    if ($nfound % 10 == 0);
					$found = 1;
					$nfound++;
				} else {
					print("%");
					print("\nDUPE: '${key}' => '${result}'\n")
					    if ($ndupes < 10);
					$ndupes++;
				}
			}

			@reskeys = $keyfun->($result);
			unless (grep {$_ eq $key} @reskeys) {
				print("*");
				print("\nEXTRA: '${key}' => '${result}'\n")
				    if ($nextras < 10);
				$nextras++;
			}
		}

		unless ($found) {
			print("-");
			print("\nMISSING: '${key}'\n")
			    if ($nmissing < 10);
			$nmissing++;
		}
		close(RESULTS);

		$nkeys++;
	}

	$nentries++;
	print("\n${nentries} entries checked...\n")
	    if ($nentries % 1000 == 0);
}

print("DONE\n");
print("ENTRIES: ${nentries}\n");
print("TOTAL KEYS: ${nkeys}\n");
print("FOUND: ${nfound} MISSING: ${nmissing} DUPES: ${ndupes} EXTRAS: ${nextras}\n");

($nmissing > 0) && exit(1);

