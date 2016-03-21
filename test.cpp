void Server::client_handle(int fd)
{
string incompleteLine = "";
int quit = 0; //flag to indicate that quit request is received
while(true)
{
char buf[1024];
string incomingData = ""; //to hold the data coming from socket
int len = recv(fd, buf, 1024, 0);
for(int i = 0; i < len; i++)
{
incomingData = incomingData + buf[i];
}
//output the data received from the socket
cout<<"##### RECEIVED LINE ######\n"+incomingData<<endl<<endl;
//check whether the incoming data has "\n" or not
	if(incomingData.find_first_of("\n")==string::npos)
	{
	//add the incoming data to incompleteLine variable to be used on later and to handle poor pipelining
	incompleteLine += incomingData;
	cout<<"incomplete line "+incompleteLine<<endl;
	}
	else
	{
//cout<<"\n incoming string length "<<incomingData.size()<<endl;
//cout<<"\n incoming string "<<incomingData<<endl;
cout<<"incoming line"+incomingData<<endl;
//bool doesNotHaveNewLine = false;
string str;
//check if the incoming is just "\n", if yes then set str as incompleteLine to be processed
if(incomingData=="\n"){
str = incompleteLine;
incompleteLine = "";
}
else{
str = incomingData.substr(0, incomingData.find_first_of("\n"));
incomingData = incomingData.substr(incomingData.find_first_of("\n") + 1); /////////////////////////////////
}
//if there is some incompleteLine then add it before the streaming data to be processed
if(incompleteLine.size()!=0){
str = incompleteLine+str;
incompleteLine = "";
}
while(str != "")
{
//processing set commands
if((str.substr(0,str.find_first_of(" "))) == "set")
{
str = str.substr(str.find_first_of(" ")+1);
//cout<<"key value pair"<<str<<endl;
string key = str.substr(0,str.find_first_of(" ")); //extracting the key
cout<<"\nkey is "<<key<<endl;
str = str.substr(str.find_first_of(" ")+1);
string val = str;//str.substr(0,str.find_first_of("\n")); ///////////////
cout<<"\nval is "<<val<<endl;
//acquiring write lock
pthread_rwlock_wrlock(&rwlock);
key_val[key] = val;
pthread_rwlock_unlock(&rwlock); //releasing the lock
//setting the response
string rsp = key + '=' + val + '\n';
//send the response for set command back to the client
for(unsigned int i = 0; i < rsp.size(); i++)
buf[i] = rsp[i];
//cout<<"\nresponse is "<<rsp<<endl;
send(fd, buf,rsp.size(), 0);
}
//processing get commands
else if((str.substr(0,str.find_first_of(" "))) == "get")
{
str = str.substr(str.find_first_of(" ")+1);
//str = str.substr(0,str.find_first_of("\n")); ///////////////////////////
cout<<"\nkey is "<<str<<endl;
//acquiring read lock
pthread_rwlock_rdlock(&rwlock);
unordered_map<string, string>::iterator it = key_val.find(str);
//setting the response
string rsp;
if(it == key_val.end())
{
rsp = str + "=null\n";
}
else
{
rsp = str + '=' + key_val[str] + '\n';
}
pthread_rwlock_unlock(&rwlock); //releaseing the read lock
//send the response for get command back to client
cout<<"Response from get is "+rsp<<endl;
for(unsigned int i = 0; i < rsp.size(); i++)
buf[i] = rsp[i];
//cout<<"\nresponse is "<<rsp<<endl;
send(fd, buf,rsp.size(), 0);
}
//processing quit commands
else if(str=="quit")
{
//closing the connection
close(fd);
quit = 1;
break;
}
//if the incoming line is not of type "set", "get" or "quit" then handling it by preparing input for next iteration using incompleteLine
else
{
str = incompleteLine+str;
incompleteLine = "";
continue;
}
if(incomingData=="")
{
str = "";
}
else
{
//prepare str and incomingData to handle next command
if(incomingData.find_first_of("\n")==string::npos)
{ if(incompleteLine.size()==0)
{
incompleteLine += incomingData;
str = "";
}
else
{
str = incompleteLine +incomingData;
incompleteLine = "";
}
}
else
str = incomingData.substr(0, incomingData.find_first_of("\n"));
incomingData = incomingData.substr(incomingData.find_first_of("\n")+1); ////////////////////////////////
}
}
}
//exit from thread
if(quit == 1)
break;
}
}