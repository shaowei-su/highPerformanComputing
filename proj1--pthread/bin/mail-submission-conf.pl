#!/usr/bin/perl
#
# This program sends a submission confirmation email with the contents
# of the submission directory listed. It assumes that a directory exists
# that contains the student's submitted files, and furthermore that the
# directory prefix is the user's id.
#
# COMMANDLINE PARAMS: recipient subj subm_dir

$dir = ".";
$subject = "submission";
$recipient = "";


if (@ARGV < 3) {
    print STDERR "$ARGV";
    print STDERR "usage: mail-submission-conf.pl recip subject subm-dir\n";
    print STDERR "    recip - email recipient\n";
    print STDERR "  subject - email subject line\n";
    print STDERR " subm-dir - the directory containing the sumbitted files\n";
}
else {
    my  %hash;

    # initialize from command-line
    $recipient = $ARGV[0];
    $recipient =~ s/(.*\/)?(.*)_.*/$2/;
    $subject = $ARGV[1];
    $dir = $ARGV[2];

    # get a list of files in submission dir
    my $fileList = "/tmp/fileList";    

    system ("find $dir | awk -F/ '{print(substr(\$0, index(\$0, \$5)))}' > $fileList");
    open (FILES, "<$fileList");
    
    # send email confirmation
    system("echo \"LIST OF SUBMITTED FILES\": > /tmp/COMMENTS");
    while (<FILES>) {
        chomp();
        my $file = $_;
	system("echo $file >> /tmp/COMMENTS");
    }
    close(FILES);
    system("cat /tmp/COMMENTS | mail -c cs258 -s \"$subject\" $recipient");
    system("rm /tmp/COMMENTS");
    system("rm /tmp/fileList");

    # print screen confirmation
    print STDOUT "confirmation email sent to $recipient\n";
}
