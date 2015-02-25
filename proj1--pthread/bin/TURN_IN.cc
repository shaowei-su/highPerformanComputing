/*
  Copied by Phil from from /u/myros/commands
  Copied 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>

#define COMMAND "/u/cs258/bin/copy_with_permissions.sh "
#define MAIL_COMMAND "/u/cs258/bin/mail-submission-conf.pl "

 char *get_user_name(){
	struct passwd *pass  = NULL;
	char *name = NULL;
	uid_t id = getuid();
	pass = getpwuid(id);
	if (pass) {
	    name = pass->pw_name;
	}
	return name;
}

 char *get_eff_user_name(){
	struct passwd *pass  = NULL;
	char *name = NULL;
	uid_t id = geteuid();
	pass = getpwuid(id);
	if (pass) {
	    name = pass->pw_name;
	}
	return name;
}

int main(int argc, char* const argv[], char *envp[]) {   
    //    printf("Real uid: %s effective uid: %s \n", get_user_name(), get_eff_user_name());
    // a little environment cleanup for savety reasons
    setenv("PATH", "/bin:/usr/bin", 1);
    setenv("IFS", " ", 1);
    unsetenv("BASH_ENV");
    unsetenv("CDPATH");


    char** newargv = new char* [argc + 3];
    newargv[0] = argv[0];
    for (int i=1; i < argc; i++) {
	// another safety check: don't try to give me any dangerous chars here
	if (strpbrk(argv[i],"[]\\:;$()|<>&")) {
	    printf("Dangerous characters in an argument name %s - cannot be executed!!!\n", argv[i]);
	    exit(5);
	}
	newargv[i] = argv[i];
	printf("Arg %d: %s\n",i,argv[i]);
    }
    
	char copy_command[256], mail_command[256];
	char orig_username[256], username[256];
	srand((int )time(NULL));
	strcpy(orig_username,get_user_name());
	strcpy(username,orig_username);
	sprintf(username+strlen(username),"%d",rand()%10000);
    printf("User: %s\n", username);
	setreuid(geteuid(), geteuid());
	char dest[256];
	strcpy(dest,"/u/cs258/project/upload/");
	strcat(dest,username);
	
	newargv[argc] = dest;
    newargv[argc+1] = "644";
    newargv[argc+2] = "744";
    newargv[argc+3] = NULL;
    
	strcpy(copy_command,COMMAND);
	strcat(copy_command,newargv[argc-1]);
	strcat(copy_command," ");
	strcat(copy_command,newargv[argc]);
	strcat(copy_command," ");
	strcat(copy_command,newargv[argc+1]);
	strcat(copy_command," ");
	strcat(copy_command,newargv[argc+2]);
	
	//	printf("%s\n\n",copy_command);
	system(copy_command);
    //execv(COMMAND, newargv);

	//newargv[argc] = orig_username;
	//newargv[argc+1] = "submission confirmation";
	//newargv[argc+2] = dest;
	//execv(MAIL_COMMAND,newargv);
	char uname[26];
	strcpy(uname,orig_username);
	strcpy(mail_command,MAIL_COMMAND);
	strcat(mail_command,uname);
	strcat(mail_command," ");
	strcat(mail_command,"258:Submission-Confirmation");
	strcat(mail_command," ");
	strcat(mail_command,newargv[argc-1]);
	//printf("%s\n",mail_command);
	system(mail_command);
	
	//printf("Real uid: %s effective uid: %s \n", get_user_name(), get_eff_user_name());    
}
