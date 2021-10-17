#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<dirent.h>
#include<time.h>
#include<pwd.h>
#include<grp.h>
#include<termios.h>
#include<ctype.h>
#include<limits.h>
#include<fcntl.h> 
#include<locale.h>
#include<langinfo.h>
#include<stack>
#include<iostream>
#include<fstream>
#include<cstdio>
#include<vector>
#include<bits/stdc++.h>
#include <sys/sendfile.h> 
#include<sys/wait.h>
using namespace std;

#define ABUF_INIT {NULL, 0}
#define CTRL_KEY(k) ((k) & 0x1f)

#define clear_s() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

void print_dir_contents(char*path);
void init_screen();
void refresh_screen();
int get_win_size(int*, int*);
//void get_cwd(stack<string>);
void enter_raw_mode();
void disable_raw_mode();
void reposition();
int process_keypress();
int read_inpt();
void print_dir_contents(char*path);
int save_enter();
void move_cursor(int);
string func(string);
void command_mode();
void listFilesRecursively(char*, char*);
bool search(string);
void display_dir(string);
string get_start_dir();
void set_start_dir(string);

string start_dir="";


struct configVar{

	struct termios orig_termios;
	int screen_rows;
	int screen_cols;
	
	int cx,cy;
	
};

struct configVar CV;

int fl=0;

void listFilesRecursively(char *basePath, char*fn)
{
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
        
        	if(!strcmp(dp->d_name, fn)){
        		fl=1;
        	}	
            

            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            listFilesRecursively(path,fn);
        }
    }

    closedir(dir);
}


void set_start_dir(string name){
	start_dir = name;
}

string get_start_dir(){
	return start_dir;
}


bool search(string file_name){

	char path[100];
 	string st=".";
	strcpy(path,st.c_str());
	
	char ff[1000];
	strcpy(ff, file_name.c_str());
	listFilesRecursively(path, ff);
    	return fl;
    
}


				
int remove_directory(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;

      r = 0;
      while (!r && (p=readdir(d))) {
	  int r2 = -1;
	  char *buf;
	  size_t len;

	  if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
	     continue;

	  len = path_len + strlen(p->d_name) + 2; 
	  buf = (char*)malloc(len);

	  if (buf) {
	     struct stat statbuf;

	     snprintf(buf, len, "%s/%s", path, p->d_name);
	     if (!stat(buf, &statbuf)) {
		if (S_ISDIR(statbuf.st_mode))
		   r2 = remove_directory(buf);
		else
		   r2 = unlink(buf);
	     }
	     free(buf);
	  }
	  r = r2;
      }
      closedir(d);
   }

   if (!r)
      r = rmdir(path);

   return r;
}


//finds full path as string
string func(string path_of_file){
	

	//make s a cahr *
	char t[1024];
	strcpy(t,path_of_file.c_str());
	
	//getcwd
	char f[1024];
	char*p;
	p=getcwd(f,1024);
	strcat(p,"/");
	strcat(p,t);
	
	
	if(!strcmp(t,".")){
		chdir(".");
		p=getcwd(f,1024);
		
	}
	else if(!strcmp(t,"..")){
		
		chdir("..");
		p=getcwd(f,1024);
		
	
	} else if(t[0]=='.' &&t[1]=='/'){
		
		chdir(t);
		p=getcwd(f,1024);
		
	}  else if(t[0]=='.' &&t[1]=='.' && t[2]=='/'){
		
		chdir(t);
		p=getcwd(f,1024);
	
	}else if(t[0]=='~'){
		char *fig;
		
		
		string fff=get_start_dir();
		
		char xx[1024];
		int i=1;
		
		
		while(t[i]!='\0'){
		 
			fff+=t[i++];
		}
		fff+='\0';
		
		fig=&fff[0];
	
		
		chdir(fig);
		p=getcwd(fig,1024);
		
		
		
	
	} else if(t[0]=='/'){
		chdir(t);
		p=getcwd(f,1024);
	
	}else{
			
		chdir(p);
		p=getcwd(f,1024);	
		
	}

	int i=0;
	string s="";
	
	while(p[i]!='\0'){
	
		s+=p[i++];
	}
	
	return s;
}



void reset(int y_pos){
	cout<<"hii";
	refresh_screen();
	clear_s();
	gotoxy(0,y_pos);
	cout<<"COMMAND MODE:";
	gotoxy(0,y_pos+1);
			
}

void print_file(string filename)
{
    int pid = fork();
    
    //parent
    if (pid!=0)
    {
        int res;
        waitpid(pid, &res, 0);
    }
    //child
    else
    {
        char *name_of_file;
    
        name_of_file=&filename[0];
        
        string editor="vi";
        char* ed=&editor[0];
        
        char *a[] = {ed, name_of_file, NULL};
        execvp("vi", a);
    }
}


map<char*, char> is_dir;
vector<char *> files;
int st=0, en=0;

void print_d(string pp){
	string res="";
	char*p= &pp[0];
	is_dir.clear();
	int coun_f=0,k;
	
	DIR *dir; 
	struct dirent *diread;
	struct stat s;	
	
	files.clear();
	chdir(p);
	    if ((dir = opendir(p)) != nullptr) {
	    	
		while ((diread = readdir(dir)) != nullptr) {
		    files.push_back(diread->d_name);
		    
		}
		
		int mn = en<files.size()? en: files.size();
		
		st = files.size()<CV.screen_rows ? 0: st;
		
		for(k=st; k<mn; k++){		
			auto f=files[k];

			struct stat sif;
			stat(f,&sif);
			
		    if(!stat(f,&sif)){
				
			    if( S_ISDIR(sif.st_mode))
			    {
				is_dir[f]='d';
			    }
			    else if( S_ISREG(sif.st_mode) )
			    {
				//it's a file
				is_dir[f]='f';
			    }
			    else
			    {
				//something else
				is_dir[f]='-';
			    }
			    
			    
		string ext="GB";
		long int file_size = sif.st_size;
		float file_s=(float)file_size;
		
		
		if(file_size<1024){
			ext="B";
		} else if(file_size>=1024 && file_size < 1048576){
			file_s/=1024.0;
			ext="KB";
		} else if(file_size>=104857 && file_size<1073741824){
			file_s/=(1024.0*1024.0);
			ext="MB";
		}
			    
			   printf("%-20s",f);
			   printf(" %9.2f ",file_s);
			   cout<<ext<<"\t";
			    
		    printf( (S_ISDIR(sif.st_mode)) ? "d" : "-");
		    printf( (sif.st_mode & S_IRUSR) ? "r" : "-");
		    printf( (sif.st_mode & S_IWUSR) ? "w" : "-");
		    printf( (sif.st_mode & S_IXUSR) ? "x" : "-");
		    printf( (sif.st_mode & S_IRGRP) ? "r" : "-");
		    printf( (sif.st_mode & S_IWGRP) ? "w" : "-");
		    printf( (sif.st_mode & S_IXGRP) ? "x" : "-");
		    printf( (sif.st_mode & S_IROTH) ? "r" : "-");
		    printf( (sif.st_mode & S_IWOTH) ? "w" : "-");
		    printf( (sif.st_mode & S_IXOTH) ? "x" : "-");
			    
			    cout<<"\t";
			    struct passwd *p;
			    p=getpwuid(sif.st_uid);
			    printf("%-12s", p->pw_name);
			    
			    struct group *g;
			    g=getgrgid(sif.st_gid);
			    printf("%-12s", g->gr_name);
			
			//remove newline from ctime    
	 		string c_t_noline = strtok(ctime(&sif.st_mtime), "\n");
			    cout<<c_t_noline<<"\r\n";
			    
		    }		    
			
		}
		
		closedir (dir);
	    } 	 
}




int main(){

	char c,ch;
	int prev_coun=0;
	int k,i;
	vector<string> traversed;
	vector<string>prev;
	vector<string>next;
	
	char* name_f;
	string cur="",temp="";
	
	
	enter_raw_mode();
	atexit(disable_raw_mode);
	
	init_screen();
	reposition();
	refresh_screen();
	en=CV.screen_rows;
	
	set_start_dir(func("."));
	cur=get_start_dir();
	
	//NOTEEE:::::give full path
	traversed.push_back(cur);
	prev.push_back(cur);
	next.push_back(cur);
	
	print_d(cur);
	reposition();
	
	gotoxy(1,1);	
	CV.cy=1;
	
	
	while(1){
		ch=cin.get();
		
		
		if(ch==':'){
			command_mode();
			
			chdir(&cur[0]);
			
			print_d(cur);
			init_screen();
			gotoxy(1,1);
						
		} else if(ch==65) {
			//cout<<"up_key";
			if(CV.cy>1)
				CV.cy--;
				
			gotoxy(CV.cx,CV.cy);
			
		}else if(ch==66){
			//cout<<"down_key";
			if(CV.cy<CV.screen_rows)
				CV.cy++;
				
			gotoxy(CV.cx,CV.cy);
			
		} else if(ch==13||ch==127){
			//find name
			string str="";
			int len;
			name_f = files[CV.cy-1+st];
			st=0;
			en=CV.screen_rows;
			
			if(is_dir[&name_f[0]]=='d' || ch==127){
					
					 if(!strcmp(name_f,"..") || ch==127){
						len=traversed.size()-2; 
						
						if(len>=0)
							temp=traversed[len];
						
						if(traversed.size()>1){
							traversed.pop_back(); }
						else{
							temp=cur;
							}
							
						if(prev.back()!=temp)
							prev.push_back(temp);
							
						
					} else if(!strcmp(name_f,".")){
						
						temp=cur;
					} else{				
						temp=cur+"/"+name_f;
						traversed.push_back(temp);

						prev.push_back(temp);
						
						
					}
			
				//clear screen, go to dir
				clear_s();
				init_screen();
				reposition();
				
				
				//change cur dir
				chdir(&temp[0]);
				cur=temp;
				print_d(temp);
				
				//go back to first pos
				gotoxy(CV.cx, CV.cy);
				
				
			} else if(is_dir[&name_f[0]]=='f'){
				//cout<<"False";
				//open .txt files in vi editor
				print_file(name_f);
			}
			
		
			
		} 
		//go up
		else if(ch=='k'){
			
			init_screen();
			clear_s();
			reposition();
			
			if(st>0)
				st--;
				
			if(en>CV.screen_rows)
				en--;
			
			print_d(cur);
			gotoxy(1,1);
		
		}
		//go down
		else if(ch=='l'){
			
			init_screen();
			clear_s();
			reposition();
			
			if(CV.screen_rows<files.size()){
				st++;
				en++;
				
			}
			
			print_d(cur);
			gotoxy(1,1);
		
		} 
		//right arrow
		else if(ch==67){
			
			int frontt=0;			
			
			if(next.size()>1){
				temp = next[frontt];				
				prev.push_back(temp);				
				next.erase(next.begin());
						
			//clear screen, go to dir
			clear_s();
			init_screen();
			reposition();
							
			//change cur dir
			chdir(&temp[0]);
			cur=temp;
			print_d(temp);
				
			//go back to first pos
			gotoxy(CV.cx, CV.cy);
				
			}
			
		}
		//left arrow
		else if(ch==68){
			
			int backk=prev.size()-2;
						
			
			if(prev.size()==1){
				temp=prev[0];
				next.insert(next.begin(),temp);
			}else{
				temp = prev[backk];
				next.insert(next.begin(),prev[backk+1]);
				prev.pop_back();
			}
			
			//clear screen, go to dir
			clear_s();
			init_screen();
			reposition();
				
				
			//change cur dir
			chdir(&temp[0]);
			cur=temp;
			print_d(temp);
				
			//go back to first pos
			gotoxy(CV.cx, CV.cy);		
			
		} 
		else if(ch=='q'){
			reposition();
			refresh_screen();
			clear_s();
			exit(0);
			break;
		} else if(ch=='h'){
			//goto home folder
			reposition();
			refresh_screen();
			init_screen();
			clear_s();
			
			temp=get_start_dir();
			
			//change cur dir
			chdir(&temp[0]);
			cur=temp;
			print_d(temp);
				
			//go back to first pos
			gotoxy(CV.cx, CV.cy);
		}	
	
	}
}


void reposition(){
	write(STDOUT_FILENO, "\x1b[H", 3);
}


void refresh_screen(){
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3); 
}


void enter_raw_mode(){
	if(tcgetattr(STDIN_FILENO, &CV.orig_termios) == -1){
		return;
	}
	
	atexit(disable_raw_mode);
	
	struct termios raw=CV.orig_termios;
	
	
	raw.c_lflag &= ~(ECHO|ICANON);
	raw.c_iflag &= ~(ICRNL|IXON |BRKINT|INPCK | ISTRIP); //ixon=pause transmission with ctrl-s and ctrl-q, so we stop this feature and icrl for carriage return diable
	 raw.c_cflag |= (CS8);
	raw.c_oflag &= ~(OPOST); //removes carriage ret from inpt
	if(tcsetattr(0, TCSAFLUSH, &raw)==-1) //changed 1 to 0
		return;
}

void disable_raw_mode() {

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &CV.orig_termios) == -1){
  	return;
  }
}

int get_win_size(int * rows, int* cols){
	struct winsize ws;
	 if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	 {
	    return -1;
	 } else {
	    *cols = ws.ws_col;
	    *rows = ws.ws_row;
	    return 0;
	 }
}


void init_screen(){
	CV.cx=1;
	CV.cy=1;
	
	get_win_size(&CV.screen_rows, &CV.screen_cols); 
	  	
	  	
}

void clear_line(){
	cout<<"\33[2K";
	cout<<"\r";
}

void command_mode(){

	char ch;
	int y_pos=CV.screen_rows-1;
	gotoxy(0,y_pos);
	cout<<"COMMAND MODE:";
	gotoxy(0,y_pos+1);
		
	string s;
	int enter_coun=0;
		
	while(ch=cin.get()){
		cout<<ch;
	
		s+=ch;
		
		if((int)ch == 27){
		
		
			//back to normal mode
			clear_s();
			
			break;
		}
		
		if(s=="q"){
			clear_s();
			exit(0);
			break;
		}
		
		
		if((int)ch==13){
			enter_coun++;
		}
		
		
		if(ch==' '||(int)ch==13){
			
			string s1;
			string s2;
			
			if(s=="create_file "){
				
				//do something;
				string filename="", path_of_file="";
				
				char ch1920;
				while((ch1920=cin.get())!=32){
					filename+=ch1920;
					cout<<ch1920;
				}
				filename+='\0';
				cout<<" ";
				
				while((ch1920=cin.get())!=13){
					path_of_file+=ch1920;
					cout<<ch1920;
				}
				path_of_file+='\0';
				
				string cur;
				cur=func(".");
				char* cur_p = &cur[0];
				
				string s;
				s=func(path_of_file);
				//cout<<s;
				
				char *s_s;
				s_s=&s[0];
				chdir(s_s);
				
				string s_f = "./"+filename;
				const char *create_file_path;
				create_file_path = &s_f[0];
				
			    	ofstream file_cf(create_file_path);
			    	file_cf.close();
			    	
			    	
			    	chdir(cur_p);
				
			} else if(s=="create_dir "){
				
				string dirname="",path_of_dir="";
				char ch1920;
				while((ch1920=cin.get())!=32){
					dirname+=ch1920;
					cout<<ch1920;
				}
				dirname+='\0';
				cout<<" ";
				
				while((ch1920=cin.get())!=13){
					path_of_dir+=ch1920;
					cout<<ch1920;
				}
				path_of_dir+='\0';
				
				
				string cur;
				cur=func(".");
				char* cur_p = &cur[0];
				
				string d;
				d=func(path_of_dir);

				char *d_d;
				d_d=&d[0];
				chdir(d_d);
				string d_dd = dirname;
				
				char* dd_dd=&d_dd[0];
				mkdir(dd_dd,0777);
				
				chdir(cur_p);

			
			}else if(s=="copy "){
			
				string cpa,cplast_str;
				int cpi,cpj,cplen;
				
				char ch1920;
				while((ch1920=cin.get())!=13){
					cpa+=ch1920;
					cout<<ch1920;
				}
				cpa+='\0';
				
				vector<string>cp_vec;

				string cp_curr=func(".");
				char* cp_cur= &cp_curr[0];


				string cpword = "";
				    for (auto x : cpa) 
				    {
					if (x == ' ')
					{
					    cp_vec.push_back(cpword);
					    cpword = "";
					}
					else {
					    cpword = cpword + x;
					}
				    }

				cplast_str=cpword;

				vector<string>cp_paths(cp_vec.size());
				vector<string>cplast_words;


				string name_cp,cpst45,cp_word;

				for(cpi=0;cpi<cp_vec.size();cpi++){
					name_cp=cp_vec[cpi];
					
					cpst45="";
					cp_word="";
					
					for(cpj=0;cpj<name_cp.length();cpj++){
						cp_word+=name_cp[cpj];
						
						if(name_cp[cpj]=='/'){
							cpst45+=cp_word;
							cp_word="";
						}	
							
					}
					
					cplast_words.push_back(cp_word);
					
					if(cpst45==""){
						cpst45=".";
					}
					
					cp_paths[cpi] = func(cpst45);
					chdir(cp_cur);
					
				}

				//destination folder
				string copy_path=func(cplast_str);
				char* cp_path = &copy_path[0];

				cplen=copy_path.length();


				//sources
				string cp_sources="",temp="",n_cp_path="";
				int cp_l;

				for(cpi=0;cpi<cp_vec.size();cpi++){
					cp_sources=cp_paths[cpi];
					
					cp_sources+="/";
					cp_sources+=cplast_words[cpi];
					n_cp_path=copy_path+"/";
					n_cp_path+=cplast_words[cpi];
					
					//source
					char*cpmvs = &cp_sources[0];
					
					//destination
					char*cpnmp=&n_cp_path[0];
					
					//copy file

					    int source = open(cpmvs, O_RDONLY, 0);
					    int dest = open(cpnmp, O_WRONLY | O_CREAT /*| O_TRUNC/**/, 0644);

					    // struct required, rationale: function stat() exists also
					    struct stat stat_source;
					    fstat(source, &stat_source);

					    sendfile(dest, source, 0, stat_source.st_size);
					    close(source);
					    close(dest);

					    
					}
			
			} else if(s=="search "){
				char ch1920;
				while((ch1920=cin.get())!=13){
					s1+=ch1920;
					cout<<ch1920;
				}
				s1+='\0';
				
			    	bool present = search(s1);
				if(present){
					cout<<" True";
				} else{
					cout<<" False";
				}
				
				fl=0;
				
			} else if(s=="move "){
			
							
				string a,last_str;
				int i,j,len;
				
				char ch1920;
				while((ch1920=cin.get())!=13){
					a+=ch1920;
					cout<<ch1920;
				}
				a+='\0';
				
				
				vector<string>move_vec;

				string move_curr=func(".");
				char* move_cur= &move_curr[0];


				string word = "";
				    for (auto x : a) 
				    {
					if (x == ' ')
					{
					    move_vec.push_back(word);
					    word = "";
					}
					else {
					    word = word + x;
					}
				    }

				last_str=word;

				vector<string>move_paths(move_vec.size());
				vector<string>last_words;


				string name_mv,st45,mv_word;

				for(i=0;i<move_vec.size();i++){
					name_mv=move_vec[i];
					
					st45="";
					mv_word="";
					
					for(j=0;j<name_mv.length();j++){
						mv_word+=name_mv[j];
						
						if(name_mv[j]=='/'){
							st45+=mv_word;
							mv_word="";
						}	
							
					}
					
					last_words.push_back(mv_word);
					
					
					if(st45==""){
						st45=".";
					}
					
					move_paths[i] = func(st45);
					
					//got back to cur dir
					chdir(move_cur);
					
				}

				//destination folder
				string move_path=func(last_str);
				char* m_path = &move_path[0];

				len=move_path.length();


				//sources
				string mv_sources="",temp="",n_m_path="";
				int mv_l;

				for(i=0;i<move_vec.size();i++){
					mv_sources=move_paths[i];
					 
					mv_sources+="/";
					mv_sources+=last_words[i];
					n_m_path=move_path+"/";
					n_m_path+=last_words[i];
					
					
					char*mvs = &mv_sources[0];
					char*nmp=&n_m_path[0];
					
					
					if(rename(mvs,nmp)==0){
						cout<<" Success";
					}

				}
				
				chdir(move_cur);
			
			
			} else if(s=="rename "){
			
				string a="",b="";
				char ch1920;
				while((ch1920=cin.get())!=32){
					a+=ch1920;
					cout<<ch1920;
				}
				a+='\0';
				cout<<" ";
				
				while((ch1920=cin.get())!=13){
					b+=ch1920;
					cout<<ch1920;
				}
				b+='\0';
				
				
				
				char* as =&a[0];
				char *bs=&b[0]; 

				if(rename(as,bs)==0){
					cout<<" success";
				}
			
			} else if(s=="delete_file "){
				
				int status,i=0;
			    	char fn[1024];

			    	
					char ch1920;
					while((ch1920=cin.get())!=13){
						fn[i++]=ch1920;
						cout<<ch1920;
						
					}
					fn[i]='\0';
			    	i=0;
			    	status = remove(fn);
			    	if(status==0)
					cout<<" delete success";
			    	
				
			
			} else if(s=="delete_dir "){

					// Use str,str1 as your directory path where it's files & subfolders will be deleted.

					string s4;
					char ch1920;
					while((ch1920=cin.get())!=13){
						s4+=ch1920;
						cout<<ch1920;
					}
					s4+='\0';
					
					string cur;
					cur=func(".");
					char* cur_p = &cur[0];
				      	
				      	string p=func(s4);
				      	char *p4;
				      	p4=&p[0];
					
				       
				       if(!remove_directory(p4)){
						cout<<" success";
					}
					
					chdir(cur_p);
			
			}else if(s=="goto "){
			
				string ss;
				char ch1920;
				while((ch1920=cin.get())!=13){
					ss+=ch1920;
					cout<<ch1920;
				}
				ss+='\0';
				
				//make s a cahr *
				
				string goto_s = func(ss);
				char* goto_s4;
				goto_s4 = &goto_s[0];
				
			}
					
			if(enter_coun==1){
				clear_line();
				enter_coun=0;
			}
			
		s="";
		
		}
		
	}
	

}