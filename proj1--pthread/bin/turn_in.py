#!/usr/bin/python
#
# Turn-in script for CSC 258 seminar summaries
# Written 2/2013 by Brandon D. Shroyer
#
# To execute, run the command
#     python turn_in.py <file_name.pdf>
#
# This script copies the target directory to a target directory owned by cs258. The PID 
# of this process will be appended to the file name to prevent multiple executions from 
# overwriting each other.
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
from shutil import copy, copytree
from os import environ, system, getpid, listdir, mkdir, chmod
from os.path import isdir, abspath, basename
from stat import S_IRWXU, S_IRGRP, S_IXGRP, S_IROTH, S_IXOTH

# Appends a forward slash to any directory names that don't have one.
def directorize(str):
    if (not isdir(str)) or (str[-1] == '/'):
        return str
    else:
        return str + "/"
        

# Recursively copies a directory while setting the permissions to allow
# The 
def clean_copy(src, dest):
    #
    print src
    #
    # mask combines all the new permission flags.
    mask = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH 
    dest_dir = directorize(dest)
    #
    # Create the new destination directory.
    mkdir(dest_dir)
    chmod(dest_dir, mask)
    #
    # loops over the contents of src.  Files are simply copied and their
    # permissions are changed; directories are subject to a recursive call.
    for elt in listdir(src):
        abs_elt = directorize(src) + elt    # The absolute path of the file.
        if not isdir(abs_elt):
            print abs_elt
            copy(abs_elt, directorize(dest) + elt)
            chmod(dest_dir, mask)
        else:
            clean_copy(abs_elt, directorize(dest_dir) + elt)
            
        
    
#============================ MAIN ================================

# The destination directory--changes with each seminar.
SUMMARY_DIR = "/u/cs258/project/upload/"

# TA information--changes with each term.
ta_name = "Brandon Shroyer"
ta_email = "bshroyer@cs.rochester.edu"

unlocked = True

if unlocked:
    # Verifies tha the appropriate number of command-line arguments is used.
    if len(argv) != 2:
        print "Assignment submission error: improper number of arguments."
        print "Proper usage: python turn_in_submit.py <dir_name>."
        exit(0)

    # Cache arguments and environment variables in more user-friendly names.
    source_dir = abspath(argv[1])
    user_name = environ["LOGNAME"]
    dest_dir = SUMMARY_DIR + user_name + "_" + str(getpid())
    #
    # Die if the source file is a directory.
    if not isdir(source_dir):
        print "Turn in submission error: '" + source_dir + "' must be a directory."
        exit(0)
    #
    # ****** COPY THE SOURCE FILE TO ITS DESTINATION ******
    #copytree(source_dir, dest_dir)
    clean_copy(source_dir, dest_dir)
    #
    # Set the attributes for the student and the TA.
    #   IMPORTANT: the message text MUST end in a new line (\n), otherwise the mail command
    #   will sit on standard input until Ctrl-D is pressed.
    #
    subject = "[CSC 258] Assignment Submission Confirmation"
    ta_message_text = user_name + " has submitted the assignment '" + dest_dir + "'.\n"
    student_message_text = "The assignment has been submitted.  If you have any questions, please contact your fearless leader at " + ta_email + ".\n"
    #
    # ****** SEND CONFIRMATION MESSAGES TO STUDENT AND TA.
    system('echo "' + student_message_text + '" | mail -s "' + subject + '" ' + user_name)
    system('echo "' + ta_message_text + '" | mail -s "' + subject + '" ' + ta_email)
    #
    print "Done."
    
else:
    user_name = environ["LOGNAME"]
    subject = "[CSC 258] Assignment Submission Rejection"
    ta_message_text = user_name + " submitted a late assignment.  J'accuse!.\n"
    student_message_text = "Assignment submission is closed.  To submit your assignment late, please contact Prof. Dwarkadas.\n"
    #
    # ****** SEND CONFIRMATION MESSAGES TO STUDENT AND TA.
    system('echo "' + student_message_text + '" | mail -s "' + subject + '" ' + user_name)
    system('echo "' + ta_message_text + '" | mail -s "' + subject + '" ' + ta_email)
    print "Assignment submission is closed.  To submit your assignment late, please contact Prof. Dwarkadas."
