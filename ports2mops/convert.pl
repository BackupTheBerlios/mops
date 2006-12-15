#!/usr/bin/perl
# $Id: convert.pl,v 1.4 2006/12/15 12:36:00 adiakin Exp $

my $MAKEFILE = 'Makefile';
my $DESCRIPTION = 'pkg-descr';
my $FILESLIST	= 'pkg-plist';
my $G_DEBUG = 1;
my $no_file = 0;

# global hash for pkg_info
my %INFO;
# default values 
$INFO{'arch'} = 'i386';
$INFO{'m_name'} = 'Some Author...';
my @files = ();

sub read_makefile {
    open(MK, $MAKEFILE) or die "Cannot open Makefile!";
    my @content = <MK>;
    close(MK);
    
    print "reading Makefile\n";    
    
    my $line;
    foreach $line ( @content ) {
	chop( $line );
	
	if ( $line =~ m/PORTNAME\?=(.*)/g ) {
	    $n = $1;
	    $n =~ s/\s+//g;;
	    $INFO{'p_name'} = $n;
	    print("p_name = " . $n . "\n");
	} 	
	if ( $line =~ m/PATCHLEVEL=(.*)/g ) {
	    $n = $1;
	    $n =~ s/\s+//g;
	    $INFO{'p_patch_level'} = $n;
	    print('p_patch_level = ' . $n . "\n");
	}
	
	if ( $line =~ m/PORTVERSION=(.*)/g ) {
	    $n = $1;
	    $n =~ s/\s+//g;
	    $n =~ s/\${PATCHLEVEL}/$INFO{'p_patch_level'}/g;
	    $INFO{'p_version'} = $n;
	    print('version = ' . $n . "\n");
	} 
	
	if ( $line =~ m/MAINTAINER\?=(.*)/g ) {
	    $n = $1;
	    $n =~ s/\s+//g;
	    $INFO{'maint'} = $n;
	    print('maintainer = ' . $n . "\n");
	}
	
	if ( $line =~ m/COMMENT\?=(.*)/g ) {
	    $n = $1;
	    $n =~ s/^\s+//g;
	    $INFO{'comment'} = $n;
	    print('comment = ' . $n . "\n");
	}
	
	
    }
}

sub read_description_file {
    if ( !open( DESCR, $DESCRIPTION) ) {
		$INFO{'descr'} = '';
    } else {
		@d = <DESCR>;
		close(DESCR);
	
		foreach $line ( @d ) {
	    	$line =~ s/^\s+//g;
	    	$INFO{'descr'} .= $line;
	    }
	}
}

sub read_files_list {
    if (!open ( FLIST, "$FILESLIST")  ) {
	$no_file = 1;
    } else {
	my $line;
	$no_file = 0;
	my @d = <FLIST>;
	foreach $line ( @d) {
	    chop($line);
	    if ( !($line =~ m/^@/g) ) {
		push( @files, $line);	
	    }
	}
    }
}

sub debug {
    if ( $G_DEBUG == 1 ) {
	print $_[0];
    }
}

sub main
{
    &read_makefile();
    &read_description_file();
    &read_files_list();
    
    my $out_file;
    
    if ( $INFO{'p_name'} ne '' ) {
	$out_file = $INFO{'p_name'} . '-port.xml';
    }
    
    if ( -r "template.xml" ) {
      if ( -e "out/" ) {
	system("cp template.xml out/$out_file");
      } else {
	system("mkdir out/");
	system("cp template.xml out/$out_file");
      }
    } else {
	die("cannot open template.xml!\n");
    }
    
    open( OUT, "template.xml") or die "cannot open template\n";
    open( DEST, ">out/$out_file") or die "cannot open out file: $out_file!\n";
    
    @ff = <OUT>;
    foreach $line ( @ff ) {
		$line =~ s/__name__/$INFO{'p_name'}/g;
		$line =~ s/__version__/$INFO{'p_version'}/g;
		$line =~ s/__arch__/$INFO{'arch'}/g;
		$line =~ s/__buid__/$INFO{'p_patch_level'}/g;
		$line =~ s/__short_descr__/$INFO{'comment'}/g;
		$line =~ s/__description__/$INFO{'descr'}/g;
		$line =~ s/__m_name__/$INFO{'m_name'}/g;
		$line =~ s/__m_email__/$INFO{'maint'}/g;


 		if ( $line =~ m/__files__/ && $no_file == 0 ) {
			print "line was $line\n";
			#$line =~ m/|(\w+)|/g;
	 		# begin dump of file list
			$t = $1;
			$t = "file";
			print "element = $t\n";
 			my $element = $t;
	 	  	my $element_start_tag = "<" . $t . ">";
 		    my $element_end_tag = "</" . $t . ">\n";
		    my $temp = '';
		
	  		foreach $f ( @files ) {
				print "processing $f...\n";
 	  			$temp .= "\t\t" . $element_start_tag . $f . $element_end_tag;
			}
			print "temp line:\n$temp\n";
			$line =~ s/__files__/$temp/g;
			$no_file = 1;
 		} 


		print DEST $line;
    }
    
    close DEST;
    close OUT;
    
}

&main();
