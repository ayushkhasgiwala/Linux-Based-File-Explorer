#include <bits/stdc++.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <fstream>

#define winds 3
#define cls() printf("\033[H\033[J")

using namespace std;



stack<string> backpath;
stack<string> fwdpath;
vector<string> filelist;
string home;
string currpath;
struct termios oldt, newt;
int low,high,cursorpos,maxpos,maxcpos=0;
static int peek_char = -1;
vector<string> files;
vector<string> directories;
int searchflag=0;
//string cmd;
void display(int low, int high,vector<string> &filelist);
void listd(string s);
void opendirectory();


void noncanonical()
{
	tcgetattr(0,&oldt);
	newt=oldt;
	newt.c_lflag &= ~ICANON;
	newt.c_lflag &= ~ECHO;
	//cout<<"hello";
	newt.c_cc[VMIN]=1;
	newt.c_cc[VTIME]=0;
	//cout<<"bye";
	tcsetattr(0,TCSANOW,&newt);
	return;
}

void gotoxy(int x,int y)
{
    printf("%c[%d;%df",0x1B,x,y);
    fflush(stdout);
} 

void canonical()
{
	tcsetattr(0,TCSANOW,&oldt);
	return;
}

string get_pwd(){
    char buff[FILENAME_MAX];
    getcwd(buff,FILENAME_MAX);
    string pwd(buff);
    return pwd;
}

void printstatus(string sp)
{
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	gotoxy(w.ws_row-4,1);
	cout<<"Status is :-";
	gotoxy(w.ws_row-4,14);
	printf("\033[K");
	cout<<sp;
	gotoxy(w.ws_row-6,17);
	printf("\033[K");
}



int readCh()
{
	char c;
	if(peek_char!=-1)
	{
		c=peek_char;
		peek_char=-1;
		//cout<<hello;
		return c;
	}
	read(0,&c,1);
	return c;
}

int keyip()
{
	char ch;
	int nread;
	if(peek_char!=-1) return 1;
	//cout<<".....";
	newt.c_cc[VMIN]=0;
	tcsetattr(0,TCSANOW,&newt);	
	nread=read(0,&ch,1);

	newt.c_cc[VMIN]=1;
	tcsetattr(0,TCSANOW,&newt);
	//cout<<"idhar aaya";
	if(nread==1)
	{
		peek_char=ch;

		return 1;
	}
	//cout<<"hello";
	return 0;
}

vector<string> parser(string cmd, char delim)
{
	vector<string> res;
	int n=cmd.length();
	string temp="";
	for(int i=0;i<n;i++)
	{
		if(cmd[i]==delim)
		{
			res.push_back(temp);
			temp="";
		}
		else
		{
			temp+=cmd[i];
		}
	}
    if(temp[0]=='~')
	{
		temp[0]='.';
		res.push_back(temp);
	}
	else if(temp[0]=='/')
	{
		res.push_back("."+temp);
	}
	//res.push_back(temp);
	//return res;
	else 
		res.push_back(temp);

	return res;
    
}

void createfile(const char *loc)
{
    int status= open(loc,O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(status==-1)
    {
        printstatus("Error : File already exists....");
    }
    else{
        printstatus("Success : File created Successfully");
    }
}

void createdirectory(const char* loc)
{
    int status=mkdir(loc,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ); 
    if(status==-1)
    {
        printstatus("Error : Directory Already Exists");
    }
    else{
        printstatus("Success : Directory Created Successfully");
    }
}





void deletefile(const char* loc)
{
	ifstream file(loc);
	bool exist=file.good();
	file.close();
	if(exist==true)
	{
		int status=remove(loc);
		if(status==0)
		{
			printstatus("Success : File Deleted");
		}
		else printstatus("Error : Cannot delete file");
	}
	else printstatus("Error : File does not exist");
}



//delete directory
void deletedir(char* loc)
{
	DIR *dp;
	struct dirent *dnp;
	//dp=opendir(path);
	if((dp=opendir(loc))==NULL)
	{
		printstatus("Error : Cannot open directory");
		return;
	}
	while((dnp=readdir(dp))!=NULL)
	{
		string nm=string(dnp->d_name);		
		if((nm=="..") || (nm=="."))
		{
			continue;
		}
			
		string temp=string(loc)+"/"+nm;			
		char *path;
		if(dnp->d_type==DT_DIR)
		{
			deletedir(const_cast<char*>(temp.c_str()));			
		}
		else
		{
			remove(const_cast<char*>(temp.c_str()));		
		}	
	}
	closedir(dp); 
	rmdir(loc);
}


//copy files
void copyfile(const char* src, const char* dest)
{
	struct stat sl;
	//check if the file already exist on the destination location
	if(stat(dest,&sl)==-1)
	{
		char buffer[BUFSIZ];
		size_t size;
		
		int s=open(const_cast<char*>(src),O_RDONLY);
		int d=open(const_cast<char*>(dest),O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		while((size=read(s,buffer,BUFSIZ))>0)
		{
			write(d,buffer,size);
		}
		close(s);
		close(d);
		
		printstatus("File is copied successfully");
		
	}
	else
		printstatus("Error : File with the same name exist at the destination");
}


void get_files_dir(string src)
{
	DIR* dp;
	struct stat sl;
	struct dirent* dnp;
	if((dp=opendir(src.c_str()))==NULL)
	{
		printstatus("Error : cannot open directory");
		return;
	}

	chdir(src.c_str());
	while((dnp=readdir(dp))!=NULL)
	{
		stat(dnp->d_name,&sl);
		if(S_ISREG(sl.st_mode))
		{
			string path=string(realpath(dnp->d_name,NULL));
			files.push_back(path);
		}
		else if(S_ISDIR(sl.st_mode))
		{
			if((string(dnp->d_name)=="..") || (string(dnp->d_name)=="."))
			{
				continue;
			}
			string path=string(realpath(dnp->d_name,NULL));
			directories.push_back(path);
			get_files_dir(dnp->d_name);
		}
	}
	chdir("..");
	closedir(dp);
}



//copy directory
void copydir(string src, string dest)
{
	string absdest=string(realpath(dest.c_str(),NULL));
	string abssrc=string(realpath(src.c_str(),NULL));
	vector<string> temp=parser(abssrc,'/');
	int x=temp.size();
	get_files_dir(abssrc);

	int indx=abssrc.find_last_of('/');
	string sname=abssrc.substr(indx+1);
	string finalloc=string(absdest+"/"+sname);
	
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	/*gotoxy(w.ws_row-1,1);
	cout<<"hello  "<<finalloc<<" "<<temp[x-1] ;
	gotoxy(w.ws_row-6,17);*/
	struct stat sl;
	//check if the directory already exist in the destination location
	if(stat(finalloc.c_str(),&sl)==-1)
	{
		mkdir(finalloc.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

		int dn=directories.size();
		for(int i=0;i<dn;i++)
		{
			vector<string> k=parser(directories[i],'/');
			string path=absdest+"/"+directories[i].substr(indx+1);
			mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
		}

		int fn=files.size();
		for(int i=0;i<fn;i++)
		{
			vector<string> k=parser(files[i],'/');
			string path=absdest+"/"+files[i].substr(indx+1);
			copyfile(files[i].c_str(),path.c_str());
		}

		printstatus("Directory copied successfully");
	}
	else printstatus("Directory already exist at the current location"); 
}

//searching

void searching(const char* src, const char* dest)
{
	DIR* dp;
	struct dirent* dnp;
	struct stat sl;
	/*struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	gotoxy(w.ws_row-1,1);
	cout<<src;
	gotoxy(w.ws_row-6,17);*/
	//return;
	
	if((dp=opendir(src))==NULL)
	{
		printstatus("Error : cannot open directory");
		return;
	}
	
	chdir(src);
	while((dnp=readdir(dp))!=NULL)
	{
		lstat(dnp->d_name,&sl);
		if(S_ISREG(sl.st_mode))
		{
			if(string(dnp->d_name)==string(dest))
			{
				printstatus("True : The file is present .");
				searchflag=1;
				return;
			}
		}
		else if(S_ISDIR(sl.st_mode))
		{
			if(string(dnp->d_name)==".." || string(dnp->d_name)==".")
				continue;

			if(string(dnp->d_name)==string(dest))
			{
				printstatus("True : The directory is present .");
				searchflag=1;
				return;
			}
			string s=string(src)+"/"+string(dnp->d_name);
			searching(s.c_str(),dest);
		}
	}
	chdir("..");
	closedir(dp);
}



//for the execution of the commands;
void executecommand(string cmd)
{
	bool validcmd=false;
    if(cmd.length()<=3)
	{
		//struct winsize w;
    	//ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		//gotoxy(w.ws_row-1,1);
		//cout<<"hello";
		//gotoxy(w.ws_row-6,17);
		printstatus("Error - Insufficient input;");
		return;
	}
    vector<string> tokens = parser(cmd, ' ');
    
    //create_file
    if(tokens[0]=="create_file")
    {
        validcmd=true;
        int n=tokens.size();
        for(int i=1;i<n-1;i++)
        {
            string base=tokens[n-1];
            string dest=base+"/"+tokens[i];
            createfile(dest.c_str());
        }
    }

    //create_directory
    if(tokens[0]=="create_dir")
    {
        validcmd=true;
        int n=tokens.size();
        for(int i=1;i<n-1;i++)
        {
            string base=tokens[n-1];
            string dest=base + "/" + tokens[i];
            createdirectory(dest.c_str());
        }
    }

	//delete_file
	if(tokens[0]=="delete_file")
	{
		
		deletefile(tokens[1].c_str());
	}

	//delete directory
	if(tokens[0]=="delete_dir")
	{
		deletedir(const_cast<char*>(tokens[1].c_str()));
		//rmdir(tokens[1].c_str());
		printstatus("Directory Deleted Successfully");
	}

	//copy files and directories
	if(tokens[0]=="copy")
	{
		int n=tokens.size();
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		/*gotoxy(w.ws_row-2,1);
		cout<<"copy me";
		gotoxy(w.ws_row-6,17);*/
		for(int i=1;i<n-1;i++)
		{
			struct stat sl;
			
			string path=get_pwd();
			path+=("/"+tokens[i]);
			stat(const_cast<char*>(path.c_str()),&sl);
			if(S_ISREG(sl.st_mode))
			{

				string dest=tokens[n-1]+"/"+tokens[i];
				
				copyfile(const_cast<char*>(path.c_str()),const_cast<char*>(dest.c_str()));
			}

			else if(S_ISDIR(sl.st_mode))
			{
				//copydir()
				/*ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
				gotoxy(w.ws_row-1,1);
				cout<<"hello in directiory";
				gotoxy(w.ws_row-6,17);*/
				copydir(path,tokens[n-1]);
				/*ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
				gotoxy(w.ws_row-1,1);
				cout<<"hello in directiory";
				gotoxy(w.ws_row-6,17);*/
			}
		}
	}


	//rename 
	if(tokens[0]=="rename")
	{
		string oldname=tokens[1];
		string newname=tokens[2];
		int res=rename(oldname.c_str(),newname.c_str());
		if(res==0)
			printstatus("Success : the rename is successful");

		else printstatus("Error : the renaming is unsuccessful");
	}


	//move
	if(tokens[0]=="move")
	{
		int n=tokens.size();
		for(int i=1;i<n-1;i++)
		{
			struct stat sl;
			string path=get_pwd();
			path+=("/"+tokens[i]);
			stat(const_cast<char*>(path.c_str()),&sl);
			if(S_ISDIR(sl.st_mode))
			{
				copydir(path,tokens[n-1]);
				deletedir(const_cast<char*>(tokens[i].c_str()));
			}

			if(S_ISREG(sl.st_mode))
			{
				string dest=tokens[n-1]+"/"+tokens[i];
				copyfile(const_cast<char*>(path.c_str()),const_cast<char*>(dest.c_str()));
				deletefile(tokens[i].c_str());
			}
		}
		printstatus("files moved successfully ");
	}

	//goto
	if(tokens[0]=="goto")
	{
		filelist.clear();
		listd(tokens[1]);
		printstatus("Success : to the location to = "+tokens[1]);
		//gotoflag=1;
	}

	//search
	if(tokens[0]=="search")
	{
		string curr=get_pwd();
		searchflag=0;
		searching(curr.c_str(),tokens[1].c_str());
		if(searchflag==0)
		{
			printstatus("False : the file/directory is not present");
		}
	}
}


void commandmode()
{
	struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	gotoxy(w.ws_row-6,17);
	int pos=17;
	//canonical();
	string cmd;
	//getline(cin>>ws,cmd);
	char c;
	bool execFlag=false;
	//cin>>cmd;
	//gotoxy(w.ws_row-4,1);
	//cout<<"hello";
	//out<<cmd;
	while(1)
	{
		if(keyip())
		{
			c=readCh();
			// on pressing esc
			if(c==27)
			{
				break;
			}

			if(c == 10) {
				execFlag = true;
			}
			if(execFlag)
			{
				//gotoxy(w.ws_row-2,1);
				//cout<<cmd;
				executecommand(cmd);
				cmd.clear();
				//gotoxy(w.ws_row-4,1);
                //cout<<"hello";
				
                gotoxy(w.ws_row-6,17);
				printf("\033[K");
				gotoxy(w.ws_row-7,1);
                pos=17;
    			cout<<"..........press : to enter into command mode & ESC to enter back in normal mode...........";
    			gotoxy(w.ws_row-6,1);
    			cout<<"Enter Command : ";
				gotoxy(w.ws_row-6,17);
				execFlag=false;
				//continue;
			}
			else
			{
				if(pos>17 && c==127)
				{
					gotoxy(w.ws_row-7,1);
    				cout<<"..........press : to enter into command mode & ESC to enter back in normal mode...........";
    				gotoxy(w.ws_row-6,1);
    				cout<<"Enter Command : ";
					gotoxy(w.ws_row-6,17);
					printf("\033[K");
					cmd.pop_back();
					pos=17;
					cout<<cmd;
					pos+=cmd.length();
					gotoxy(w.ws_row-6,pos);
				}
				else if(pos==17 && c==127)
				{
					continue;
				}
				else 
				{
					cout<<c;
					gotoxy(w.ws_row-6,pos+1);
					cmd.push_back(c);
					pos++;
				}
				
				
				
				
				
				
				
				
				
				
				
				//pos++;
				//gotoxy(w.ws_row-6,pos+1);
				/*cout<<c;
				gotoxy(w.ws_row-6,pos+1);
				cmd.push_back(c);
				pos++;
				//backspace
				if(pos>17 && c==127)
				{
					//gotoxy(w.ws_row-6,pos-1);
					gotoxy(w.ws_row-7,1);
    				cout<<"..........press : to enter into command mode & ESC to enter back in normal mode...........";
    				gotoxy(w.ws_row-6,1);
    				cout<<"Enter Command : ";
					gotoxy(w.ws_row-6,17);
	
					cmd.pop_back();
					cmd.pop_back();
	
					pos-=2;
					gotoxy(w.ws_row-1,1);
                	cout<<cmd;
					printf("\033[K");
					gotoxy(w.ws_row-6,17);
					cout<<cmd.c_str();
					gotoxy(w.ws_row-6,pos);
					printf("\033[K");
				}*/
			}
		}
		
	}
	//noncanonical();
}