#!/usr/bin/perl -w

use Test::Simple tests => 15;

my $nfs = "nfs-fuse";
my $path = "../fuse/";
my $cmd;

$cmd = "$nfs -V";
ok(`$path$cmd` =~ m/\d\.\d\.\d/, $cmd);

$cmd = "$nfs --version";
ok(`$path$cmd` =~ m/\d\.\d\.\d/, $cmd);

$cmd = "$nfs -h";
ok(`$path$cmd` =~ m/^usage: $nfs/, $cmd);

$cmd = "$nfs --help";
ok(`$path$cmd` =~ m/^usage: $nfs/, $cmd);

# This is a libfuse limitation, not going to fix it.
$cmd = "$nfs -h xx yy";
ok(not(`$path$cmd 2>&1` =~ m/^usage: $nfs/), $cmd);

$cmd = "$nfs";
ok(`$path$cmd 2>&1` =~ m/^$nfs: no nfs target/, $cmd);

$cmd = "$nfs xx";
ok(`$path$cmd 2>&1` =~ m/^$nfs: no mount point/, $cmd);

$cmd = "$nfs xx yy";
ok(`$path$cmd 2>&1` =~ m/^fuse: bad mount point/, $cmd);

my $mnt = "temp_mount";
`mkdir $mnt 2>&1`;
ok(-d $mnt, "$mnt exists");

$cmd = "$nfs xx $mnt";
ok(`$path$cmd 2>&1` =~ m/^$nfs: remote share not in/, $cmd);

$cmd = "$nfs -o nfsvers=1 xx:yy $mnt";
ok(`$path$cmd 2>&1` =~ m/^$nfs: unsupported NFS version/, $cmd);

$cmd = "$nfs -o nfsvers=5 xx:yy $mnt";
ok(`$path$cmd 2>&1` =~ m/^$nfs: unsupported NFS version/, $cmd);

$cmd = "$nfs -o vers=2 xx:yy $mnt";
ok(`$path$cmd 2>&1` =~ m/^$nfs: unsupported NFS version/, $cmd);

$cmd = "$nfs -o vers=6 xx:yy $mnt";
ok(`$path$cmd 2>&1` =~ m/^$nfs: unsupported NFS version/, $cmd);

`rmdir $mnt 2>&1`;
ok(not(-d $mnt), "$mnt removed");
