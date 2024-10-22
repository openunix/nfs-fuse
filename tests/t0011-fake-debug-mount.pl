#!/usr/bin/perl -w
use Time::HiRes qw(stat);
use Test::Simple tests => 7;

my $nfs = "nfs-fuse";
my $path = "../fuse/";
my $cmd;

my $testdata = "testdata";
my $testdir = `pwd`;
chomp $testdir;
$testdir = $testdir . "/" . $testdata;

my $testshare = "127.0.0.1:$testdir";
my $result = "result";

my $mnt = "temp_mount";
`mkdir -p $mnt 2>&1`;
ok(-d $mnt, "$mnt exists");

`mkdir -p $testdir 2>&1`;
ok(-d $testdir, "$testdir exists");

$cmd = "sudo exportfs -o rw,insecure,no_root_squash,subtree_check $testshare";
ok(system($cmd) == 0, $cmd);

`touch -d '3 hours ago' -a $testdir`;
`touch -d '2 hours ago' -m $testdir`;

my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
    $atime,$mtime,$ctime,$blksize,$blocks)
       = stat($testdir);

$cmd = "$path$nfs -d -f -o nfsvers=4,debug=0xFF $testshare $mnt >$result 2>&1";

ok(system($cmd) == 0, $cmd);

my $satime = `grep atime $result | sed s/nfs_fattr:\\ atime:\\ //`;
my $sctime = `grep ctime $result | sed s/nfs_fattr:\\ ctime:\\ //`;
my $smtime = `grep mtime $result | sed s/nfs_fattr:\\ mtime:\\ //`;

chomp $satime;
chomp $sctime;
chomp $smtime;

ok($satime == $atime, "atime: $satime == $atime");
ok($sctime == $ctime, "ctime: $sctime == $ctime");
ok($smtime == $mtime, "mtime: $smtime == $mtime");

`sudo exportfs -u $testshare`;
`rm -f $result`;
`rm -rf $testdir`;