#!/usr/bin/python
#
# Turn-in script for CSC 258 seminar summaries
# Written 2/2013 by Brandon D. Shroyer
#
# To execute, run the command
#     python summary_submit.py <file_name.pdf>
#
# This script copies the target file to a target directory owned by cs258. The PID 
# of this process will be appended to the file name to prevent multiple executions
# from overwriting each other.  Note that this script will reject non-PDFs.
#
# Once the file copy is finished, this script notifies the student and the TA via email.
#
# THINGS THAT NEED TO BE CHANGED EACH TERM:
#   ta_name
#   ta_email
#
# THINGS THAT NEED TO BE CHANGED AFTER EACH SEMINAR:
#   SUMMARY_DIR

import re
from sys import argv
from shutil import copy
from os import environ, system, getpid, chmod
from os.path import isfile
from stat import S_IRWXU, S_IRGRP, S_IROTH

# The destination directory--changes with each seminar.
SUMMARY_DIR = "/u/cs258/summaries/upload/"

# TA information--changes with each term.
ta_name = "Brandon Shroyer"
ta_email = "bshroyer@cs.rochester.edu"

unlocked = False

if unlocked:
    # Verifies tha the appropriate number of command-line arguments is used.
    if len(argv) != 2:
        print "Summary submission error: improper number of arguments."
        print "Proper usage: python summary_submit.py <file_name.pdf>."
        exit(0)

    # Cache arguments and environment variables in more user-friendly names.
    source_file = argv[1]
    user_name = environ["LOGNAME"]
    dest_file = SUMMARY_DIR + user_name + "_" + str(getpid()) + ".pdf"
    #
    # Die if the source file is a directory.
    if not isfile(source_file):
        print "Submission error: '" + source_file + "' must be a file."
        exit(0)

    # Do a regular expression check and die if the file does not have a .pdf extension.
    if not re.search(re.compile('\.pdf$', re.IGNORECASE), source_file):
        print "Submission error: file '" + source_file + "' is not a PDF."
        exit(0)

    # ****** COPY THE SOURCE FILE TO ITS DESTINATION ******
    copy(source_file, dest_file)
    chmod(dest_file, S_IRWXU | S_IRGRP | S_IROTH)
    #
    # Set the attributes for the student and the TA.
    #   IMPORTANT: the message text MUST end in a new line (\n), otherwise the mail command
    #   will sit on standard input until Ctrl-D is pressed.
    #
    subject = "[CSC 258] Summary Submission Confirmation"
    ta_message_text = user_name + " has submitted the summary '" + dest_file + "'.\n"
    student_message_text = "The summary '" + source_file + "' has been submitted.  If you have any questions, please contact your fearless leader at " + ta_email + ".\n"
    #
    # ****** SEND CONFIRMATION MESSAGES TO STUDENT AND TA.
    system('echo "' + student_message_text + '" | mail -s "' + subject + '" ' + user_name)
    system('echo "' + ta_message_text + '" | mail -s "' + subject + '" ' + ta_email)
    print "Done."

else:
    user_name = environ["LOGNAME"]
    subject = "[CSC 258] Summary Submission Rejected"
    ta_message_text = user_name + " has tried to submit a summary out-of-class.  J'accuse!"
    student_message_text = "Summary not accepted.  As per Sandhya's Blackboard message, summaries are now collected in-class on the due date, NO EXCEPTIONS.\n"
    #
    # ****** SEND HATE MAIL TO STUDENT AND TA.
    system('echo "' + student_message_text + '" | mail -s "' + subject + '" ' + user_name)
    system('echo "' + ta_message_text + '" | mail -s "' + subject + '" ' + ta_email)
    print "Submission rejected.  Summaries are now collected in-class on the due date, NO EXCEPTIONS."
