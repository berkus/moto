#!/usr/bin/perl

while ($ARGV[0]) {
	$file = shift(@ARGV);
	open (F, $file);
		undef $/;
		$data = <F>;
	close (F);
   $data =~ s/@//g;	
   $data =~ s/\[(.+)\]/$1 /g;	
	open (F, ">$file");
	   print F $data;
   close (F);
}
