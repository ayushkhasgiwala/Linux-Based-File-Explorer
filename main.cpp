#include "header2.h"
int q=0;

DIR  *dp;
struct dirent *dirp;




void display(int low, int high,vector<string> &filelist)
{
    cls();
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    const char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    //gotoxy(0,0);
    //cout<<"................................list of files in current directory......................"<<endl<<endl;
    //gotoxy(w.ws_row-q-1,1);
    //cout<<"inside display";
    //gotoxy(1,1);
    for(int i=low;i<high;i++)
    {
        struct stat sta;
        stat(filelist[i].c_str(),&sta);
        cout<<">  ";
       //gotoxy(w.ws_row-q-1,1);
       //cout<<"inside for"<<" "<<i<<" ";
       //gotoxy(i+1,1);
        
        //name of the file
        /*cout<<filelist[i]<<"\t";
        if(filelist[i][0]=='.')
            cout<<"\t";*/

        //size
        long long sz=sta.st_size;
        //gotoxy(w.ws_row-q-2,1);
        //cout<<"inside for"<<" "<<i<<" ";
        //gotoxy(i+1,1);
        char unit='B';
        if(sz>=1073741824)
        {
            sz/=1073741824;
            unit='G';
        }
        else if(sz>=1048576)
        {
            sz/=1048576;
            unit='M';
        }
        else if(sz>=1024)
        {
            sz/=1024;
            unit='K';
        }
        //gotoxy(w.ws_row-q-3,1);
       //cout<<"inside for"<<" "<<i<<" ";
       //gotoxy(i+1,1);
        cout<<sz<<unit<<"\t";

        //ownership
        // grp and user
        struct passwd *psd=getpwuid(sta.st_uid);
        const char *uname=psd->pw_name;
        cout<<uname<<"\t";

        struct group *grp=getgrgid(sta.st_gid);
        const char *gname=grp->gr_name;
        cout<<gname<<"\t";

        //permissions
        if(S_ISDIR(sta.st_mode))
            cout<<"d";
        else if(S_ISSOCK(sta.st_mode))
            cout<<"s";
        else cout<<"-";

        if(S_IRUSR & sta.st_mode)
            cout<<"r";
        else cout<<"-";

        if(S_IWUSR & sta.st_mode)
            cout<<"w";
        else cout<<"-";

        if(S_IXUSR & sta.st_mode)
            cout<<"x";
        else cout<<"-";

        if(S_IRGRP & sta.st_mode)
            cout<<"r";
        else cout<<"-";

        if(S_IWGRP & sta.st_mode)
            cout<<"w";
        else cout<<"-";

        if(S_IXGRP & sta.st_mode)
            cout<<"x";
        else cout<<"-";

        if(S_IROTH & sta.st_mode)
            cout<<"r";
        else cout<<"-";

        if(S_IWOTH & sta.st_mode)
            cout<<"w";
        else cout<<"-";

        if(S_IXOTH & sta.st_mode)
            cout<<"x";
        else cout<<"-";

        cout<<"\t";

        // last modified 
        struct tm *tim=gmtime(&(sta.st_mtime));
		cout<<months[(tim->tm_mon)]<<" "<<tim->tm_mday<<" "<<tim->tm_hour<<":"<<tim->tm_min<<"\t";

        //name of the file
        cout<<filelist[i]<<"\t";
        if(filelist[i][0]=='.')
            cout<<"\t";
        cout<<"\n";
    }
    gotoxy(w.ws_row-7,1);
    cout<<"..........press : to enter into command mode & ESC to enter back in normal mode...........";
    gotoxy(w.ws_row-6,1);
    cout<<"Enter Command : ";
    //gotoxy(high+2,1);
    //gotoxy(winds,1);
}

void listd(string s)
{
    //printf("%c[2J",27);
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    //gotoxy(w.ws_row-5,1);
    //cout<<s;
    //gotoxy(cursorpos,1);
    const char *dirname=s.c_str();
    if((dp=opendir(dirname))!=NULL)
    {
        //gotoxy(maxcpos+10,1);
        //cout<<"inside if";
        //gotoxy(cursorpos,1);
        chdir(dirname);
        while((dirp = readdir(dp))!=NULL)
        {
            string fname((dirp->d_name));
            filelist.push_back(fname);
            //gotoxy(maxcpos+11,1);
            //cout<<"inside while";
            //gotoxy(cursorpos,1);
        }
    }

    closedir(dp);
    //free(*dp);
    int x=filelist.size();
    low=0;
    if(x-1<winds)
    {
        high=x;
    }
    else high=winds;
    maxcpos=x;
    /*if(x-1<winds)
    {
        maxcpos=x-1;
    }
    else maxcpos=;*/
    //cursorpos=high-1+3;
    cursorpos=winds;
    //gotoxy(maxcpos+15,1);
    //cout<<"before display"<<" "<<low<<" "<<high<<" "<<maxcpos<<" "<<x<<" ";
    //gotoxy(cursorpos,1);
   
    display(low,high,filelist);
    return;
}

void opendirectory()
{
    string path;
    int index=low+cursorpos-1;
    string fname=filelist[index];
    struct stat sta1;
    stat(fname.c_str(),&sta1);
    string prev=get_pwd();
    //if it is a directory
    if(S_ISDIR(sta1.st_mode))
    {
        
        
        if(fname==".")
            return ;
        if(fname=="..")
        {
            if(backpath.size()>0)
            {
                string temp=fwdpath.top();
                fwdpath.pop();
                backpath.push(temp);
                path="../";
            }
        }
        else{
            path=string(realpath(fname.c_str(),NULL));
            backpath.push(prev);
            //gotoxy(maxcpos+5,1);
            //cout<<path;
            //gotoxy(cursorpos,1);
        }
        filelist.clear();
        //gotoxy(maxcpos+6,1);
        //cout<<"after clear";
        //gotoxy(cursorpos,1);
        low=0;
        high=min(winds,maxcpos);
        listd(path);
        //gotoxy(maxcpos+7,1);
        //cout<<"after list d";
        //gotoxy(cursorpos,1);
        int y=filelist.size();
        gotoxy(min(winds,y),1);
        return;
    }

    //if it is the file
    //if(S_ISREG(sta1.st_mode))
    else
    {
        //gotoxy(maxcpos+5,1);
        //cout<<fname;
        //gotoxy(cursorpos,1);
        pid_t pid;
        pid=fork();
        if(pid==0)
        {
            execl("/usr/bin/xdg-open","xdg-open",fname.c_str(),NULL);
        }
    }
    return;
}




int main()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); 
    noncanonical();
    currpath=get_pwd();
    home=currpath;
    listd(currpath);
    //fwdpath.push(home);
    
    gotoxy(winds,1);
    //gotoxy(1,1);
    char c;
    while(1)
    {
        if(keyip())
        {
            c=readCh();
            //down arrow key
            if(c==66)
            {
                if(cursorpos<winds)
                {
                    cursorpos++;
                    gotoxy(cursorpos,1);
                }
                else
                {
                    if(high<maxcpos)
                    {
                        low++;
                        high++;
                        display(low,high,filelist);
                        gotoxy(winds,1);
                    }
                }
            }

            //up arrow
            if(c==65)
            {
                if(cursorpos>1)
                {
                    cursorpos--;
                    gotoxy(cursorpos,1);
                }
                else{
                    if(low!=0)
                    {
                        low--;
                        high--;
                        cursorpos=1;
                        display(low,high,filelist);
                        gotoxy(cursorpos,1);
                    }
                } 
            }

            //k=vertical overflow ----neche kiskega
            if(c==107)
            {
                if(high+winds<maxcpos)
                {
                    low=high;
                    high=high+winds;
                    display(low,high,filelist);
                    gotoxy(cursorpos,1);
                }
                else{
                    //int diff=maxcpos-high;
                    //low=low+diff+1;
                    high=maxcpos;
                    low=high-winds;
                    display(low,high,filelist);
                    gotoxy(cursorpos,1);
                }
            }

            // l---vertical overflow----upar kiskega
            if(c==108)
            {
                if(low-winds>=0)
                {
                    high=low;
                    low=low-winds;
                    display(low,high,filelist);
                    gotoxy(cursorpos,1);
                }
                else{
                    low=0;
                    high=winds;
                    display(low,high,filelist);
                    gotoxy(cursorpos,1);
                }
            }

            //on pressing enter
            if(c==10)
            {
                opendirectory();
            }

            //left arrow
            if(c==68)
            {
                string path;
                if(backpath.size()>0)
                {
                    string current=get_pwd();
                    string prev=backpath.top();
                    fwdpath.push(current);
                    backpath.pop();
                    //backpath.push(current);
                    filelist.clear();
                    listd(prev);
                    gotoxy(winds,1);
                }
            }

            //right arrow
            if(c==67)
            {
                string path;
                if(fwdpath.size()>0)
                {
                    string current=get_pwd();
                    string next=fwdpath.top();
                    backpath.push(current);
                    fwdpath.pop();
                    filelist.clear();
                    listd(next);
                    gotoxy(winds,1);
                }
            }

            //backspace or one level up
            if(c==127)
            {
                string path="../";
                string current=get_pwd();
                if(current!=home)
                {
                    backpath.push(current);
                    filelist.clear();
                    listd(path);
                    gotoxy(winds,1);
                }
            }


            //home (h) key
            if(c==104)
            {
                string path=home;
                string current=get_pwd();
                backpath.push(current);
                filelist.clear();
                listd(path);
                gotoxy(winds,1);
            }

            //: for entering into command mode
            if(c==':')
            {
                struct winsize w;
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	            //gotoxy(w.ws_row-6,17);
                commandmode();
                gotoxy(w.ws_row-2,1);
                cout<<"hello;";
                //string path=backpath.top();
                filelist.clear();
                listd(home);
                gotoxy(winds,1);
            }

            //to quit the program
            if(c=='q')
            {
                gotoxy(w.ws_row-1,1);
                break;
            }
        }
    }
    canonical();
}