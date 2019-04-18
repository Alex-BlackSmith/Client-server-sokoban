#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <map>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT1 "50001"
#include "TwoDimArray.h"
#include <fstream>
#include <ostream>
#include <vector>
#include <libtcod.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#define BACKLOG 10   // how many pending connections queue will hold
#define MAXBUFFERSIZE 1000000

using std::ifstream;
using std::string;
using std::vector;
using std::map;

bool isMapSent1 = false, isMapSent2 = false; //flag that shows is our map sent or not
bool *isMapSentPtr1 = &isMapSent1, *isMapSentPtr2 = &isMapSent2; //pointer to variable above

//That function fill all structures for the first time.
void firstFill(const TwoDimArray<char>& twoDArray, vector<int>& tmpPlrPos1, vector<int>& tmpPlrPos2, map<vector<int>,char>& mapOfWinPositions);

//That function check char from recieved buffer and change object positions in structure
void chkKeyKeyAndMovePlayer(const char& key ,vector<int>& tmpPlrPos, TwoDimArray<char>& twoDimArray, const char& PlayerChar);

//That function updates vector containing win positions
void updateWinPositions(TwoDimArray<char>& twoDArray, map<vector<int>,char>& mapOfWinPositions);

void ConstructAndSendBuffer(const int socket_desctriptor, TwoDimArray<char>& TDA);

//That function check WIN condition for the current structure
bool ifWin(map<vector<int>,char>& mapOfWinPositions);

//That fucntion processes appropriate ip4 or ip6 address
void *get_approp_addr(struct sockaddr *sock_a);

//That function return random file
string GetRandomFileByPath(const char* user_path);

int main(void)
{
    
    TwoDimArray<char> Test;
    vector<int> tempPlrPos1; //temporary player1 position vector (x,y)
    vector<int> tempPlrPos2; //temporary player2 position vector (x,y)
    map<vector<int>,char> mapCharWin; // vector of win positions "(x,y) - character"
    char playerChar1 = 'P'; // character denotes 1-st player
    char playerChar2 = 'K'; // character denotes 1-st player
    const char* path = "/home/akuznetsov/Client-server-sokoban/"; //path with maps
    string fileName = GetRandomFileByPath(path); //call to function, which return full path to a random file
    
    
    
    ifstream in(fileName);
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    char buffer[size];
    file.read(buffer, size);
    if (!isMapSent2){
        in >> Test;
        firstFill(Test, tempPlrPos1, tempPlrPos2, mapCharWin); // fill structures that 
        // contains all coordinates of players and objects on map
    }
    
    char buf[MAXBUFFERSIZE];
    int sock, new_sock1,new_sock2,numbytes, addrlen, client_socket[1] ,
            max_clients = 1 , activity, i , valread , sd;
    struct addrinfo hints, *servinfo1, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char address_pres[INET6_ADDRSTRLEN];
    
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo1)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo1; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            //exit(1);
        }
        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            //close(sock);
            perror("server: bind");
            continue;
        }
        break;
    }
    
    if (listen(sock, 1) == -1) {
        perror("listen");
    }

    sin_size = sizeof their_addr;

    new_sock1 = accept(sock, (struct sockaddr *) &their_addr, &sin_size);
    fcntl(new_sock1,F_SETFL, O_NONBLOCK);
    new_sock2 = accept(sock, (struct sockaddr *) &their_addr, &sin_size);
    fcntl(new_sock2,F_SETFL, O_NONBLOCK);
    
    inet_ntop(their_addr.ss_family,
              get_approp_addr((struct sockaddr *) &their_addr),
              address_pres, sizeof address_pres);
    printf("server: got connection from %s\n", address_pres);

    printf("server: waiting for connections...\n");
    bool turn1 = true, turn2 = false;
    bool* t1 = &turn1;
    bool* t2 = &turn2;

    while(1) {

        if (new_sock1 == -1 || new_sock2 == -1) {
            perror("accept");
            continue;
        }

        if((turn1 == false) && (turn2 == false)){
            turn1 = true;
        }

        //Request for MAP to server from client n1
        if ((recv(new_sock1, buf, MAXBUFFERSIZE - 1, MSG_DONTWAIT)) == -1) {
            if ((*isMapSentPtr1)){
                ConstructAndSendBuffer(new_sock1, Test);
            }
        }
        else {
            cout << "MAP CMD FROM 1" << endl;
            if (turn1) {
                if (buf[0] == 'm' && !(*isMapSentPtr1)) {
                    if (send(new_sock1, buffer, size, 0) == -1) {
                        perror("send MAP to 1-st client");
                    }
                    *isMapSentPtr1 = !(*isMapSentPtr1);
                }
                    //Check recieved key from a client
                    if ((buf[0] == 'w') || (buf[0] == 'a')
                        || (buf[0] == 's') || (buf[0] == 'd')) {
                        //for some client move player and check win
                        chkKeyKeyAndMovePlayer(buf[0], tempPlrPos1, Test, playerChar1);
                        updateWinPositions(Test, mapCharWin);
                        
                        char *win = "win";
                        if (!ifWin(mapCharWin)) {
                            ConstructAndSendBuffer(new_sock1, Test);
                        } else {
                            ConstructAndSendBuffer(new_sock1, Test);
                            break;
                        }
                    }
                //}
                *t1 = false;
                *t2 = true;
            }
        }
        buf[0] = {};
        //Request for MAP to server from client n2
        if ((recv(new_sock2, buf, MAXBUFFERSIZE - 1, 0)) == -1) {
            if ((*isMapSentPtr2)){
                //current map send
                ConstructAndSendBuffer(new_sock2, Test);
            }
        } else {
            if (turn2) {
                if (*buf == 'm' && !(*isMapSentPtr2)) {
                    if (send(new_sock2, buffer, size, 0) == -1) {
                        perror("send MAP to 2-d client");
                    }
                    *isMapSentPtr2 = !(*isMapSentPtr2);
                }
                //Check recieved key from a client
                if ((buf[0] == 'w') || (buf[0] == 'a')
                    || (buf[0] == 's') || (buf[0] == 'd')) {
                    //for some client move player and check win
                    chkKeyKeyAndMovePlayer(buf[0], tempPlrPos2, Test, playerChar2);
                    updateWinPositions(Test, mapCharWin);
                    char *win = "win";
                    //TO_DO WIN RULE
                    if (!ifWin(mapCharWin)) {
                        ConstructAndSendBuffer(new_sock2, Test);
                    } else {
                        ConstructAndSendBuffer(new_sock2, Test);
                        break;
                    }
                }
                *t2 = false;
            }
        }
        buf[0] = {};
        usleep(1000*20);
    }
    return 0;
}

void *get_approp_addr(struct sockaddr *sock_a)
{
    if (sock_a->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sock_a)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sock_a)->sin6_addr);
}

void chkKeyKeyAndMovePlayer(const char& key ,vector<int>& tmpPlrPos, TwoDimArray<char>& twoDimArray, const char& PlayerChar){
    if ( key == 'w' || key == 'W' ) {
        if (twoDimArray.getObjPos(tmpPlrPos[1] - 1, tmpPlrPos[0]) != '#'){
            if (twoDimArray.getObjPos(tmpPlrPos[1] - 1, tmpPlrPos[0]) != 'B'){
                twoDimArray.setObjPos(tmpPlrPos[1] - 1, tmpPlrPos[0], PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0], ' ');
                tmpPlrPos = {tmpPlrPos[0], tmpPlrPos[1] - 1};
            }
            else if ((twoDimArray.getObjPos(tmpPlrPos[1] - 1, tmpPlrPos[0]) == 'B')
                     && (twoDimArray.getObjPos(tmpPlrPos[1] - 2,tmpPlrPos[0]) != '#')
                     && (twoDimArray.getObjPos(tmpPlrPos[1] - 2,tmpPlrPos[0]) != 'B')){
                twoDimArray.setObjPos(tmpPlrPos[1] - 1, tmpPlrPos[0], PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1] - 2, tmpPlrPos[0], 'B');
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0], ' ');
                tmpPlrPos = {tmpPlrPos[0], tmpPlrPos[1] - 1};
            }
        }
    }
    else if ( key == 'a' || key == 'A' ) {
        if (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] - 1) != '#'){
            if (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] - 1) != 'B'){
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0] - 1, PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0], ' ');
                tmpPlrPos = {tmpPlrPos[0] - 1, tmpPlrPos[1]};
            }
            else if ((twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] - 1) == 'B')
                     && (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] - 2) != '#')
                     && (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] - 2) != 'B')){
                twoDimArray.setObjPos(tmpPlrPos[1],tmpPlrPos[0] - 1, PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0], ' ');
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0] - 2, 'B');
                tmpPlrPos = {tmpPlrPos[0] - 1, tmpPlrPos[1]};
            }
        }
    }
    else if ( key == 's' || key == 'S' ) {
        if (twoDimArray.getObjPos(tmpPlrPos[1] + 1, tmpPlrPos[0]) != '#'){
            if (twoDimArray.getObjPos(tmpPlrPos[1] + 1, tmpPlrPos[0]) != 'B'){
                twoDimArray.setObjPos(tmpPlrPos[1] + 1, tmpPlrPos[0], PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0], ' ');
                tmpPlrPos = {tmpPlrPos[0], tmpPlrPos[1] + 1};
            }
            else if ((twoDimArray.getObjPos(tmpPlrPos[1] + 1, tmpPlrPos[0]) == 'B')
                     && (twoDimArray.getObjPos(tmpPlrPos[1] + 2, tmpPlrPos[0]) != '#')
                     && (twoDimArray.getObjPos(tmpPlrPos[1] + 2, tmpPlrPos[0]) != 'B')){
                twoDimArray.setObjPos(tmpPlrPos[1] + 1, tmpPlrPos[0], PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0],  ' ');
                twoDimArray.setObjPos(tmpPlrPos[1] + 2,tmpPlrPos[0], 'B');
                tmpPlrPos = {tmpPlrPos[0], tmpPlrPos[1] + 1};
            }
        }
    }
    else if ( key == 'd' || key == 'D' ) {
        if (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] + 1) != '#'){
            if (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] + 1) != 'B'){
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0] + 1,  PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1], tmpPlrPos[0],  ' ');
                tmpPlrPos = {tmpPlrPos[0] + 1, tmpPlrPos[1]};
            }
            else if ((twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] + 1) == 'B')
                     && (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] + 2) != '#')
                     && (twoDimArray.getObjPos(tmpPlrPos[1], tmpPlrPos[0] + 2) != 'B')){
                twoDimArray.setObjPos(tmpPlrPos[1],tmpPlrPos[0] + 1,  PlayerChar);
                twoDimArray.setObjPos(tmpPlrPos[1],tmpPlrPos[0],  ' ');
                twoDimArray.setObjPos(tmpPlrPos[1],tmpPlrPos[0] + 2,  'B');
                tmpPlrPos = {tmpPlrPos[0] + 1, tmpPlrPos[1]};
            }
        }
    }
}

void firstFill(const TwoDimArray<char>& twoDArray, vector<int>& tmpPlrPos1, vector<int>& tmpPlrPos2, map<vector<int>,char>& mapOfWinPositions){
    for (auto i = 0; i < twoDArray.getDimY(); i++){
        for (auto j = 0; j < twoDArray.getDimX(); j++){
            if (twoDArray.getObjPos(i,j) == 'P'){
                tmpPlrPos1 = {j,i};
            }
            else if (twoDArray.getObjPos(i,j) == 'K'){
                tmpPlrPos2 = {j,i};
            }
            else if (twoDArray.getObjPos(i,j) == '#'){
            }
            else if (twoDArray.getObjPos(i,j) == 'B'){
            }
            else if (twoDArray.getObjPos(i,j) == 'X'){
                mapOfWinPositions[{j,i}] = 'X';
            }
        }
    }
}

void updateWinPositions(TwoDimArray<char>& twoDArray, map<vector<int>,char>& mapOfWinPositions) {
    for (auto &cross : mapOfWinPositions) {
        if (twoDArray.getObjPos(cross.first[1], cross.first[0]) == 'X') {
            twoDArray.setObjPos(cross.first[1], cross.first[0], 'X');
            cross.second = 'X';
        } else if (twoDArray.getObjPos(cross.first[1], cross.first[0]) == 'B') {
            twoDArray.setObjPos(cross.first[1], cross.first[0], 'B');
            cross.second = 'B';
        } else if (twoDArray.getObjPos(cross.first[1], cross.first[0]) == 'P') {
            twoDArray.setObjPos(cross.first[1], cross.first[0], 'P');
            cross.second = 'P';
        } else if (twoDArray.getObjPos(cross.first[1], cross.first[0]) == 'K') {
            twoDArray.setObjPos(cross.first[1], cross.first[0], 'K');
            cross.second = 'K';
        } else if (twoDArray.getObjPos(cross.first[1], cross.first[0]) == ' ') {
            twoDArray.setObjPos(cross.first[1], cross.first[0], 'X');
            //TCODConsole::root->setCharBackground(cross.first[0], cross.first[1], colVec[3]);
        }
    }
}

string GetRandomFileByPath(const char* user_path){
    srand( time(0) );
    DIR *dir;
    vector<char*> vecOfFiles;
    string path_to_return = user_path;
    struct dirent *ent;
    if ((dir = opendir (user_path)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            char* ch = ent -> d_name;
            string tr = ent -> d_name;
            char type[3];
            type[0] = ch[tr.length() - 7];
            type[1] = ch[tr.length() - 6];
            type[2] = ch[tr.length() - 5];
            type[3] = ch[tr.length() - 4];
            type[4] = ch[tr.length() - 3];
            type[5] = ch[tr.length() - 2];
            type[6] = ch[tr.length() - 1];
            if((type[0] == 'm') && (type[1] == 'a') && (type[2] == 'p')
               && (type[3] == '.')
               && (type[4] == 't') && (type[5] == 'x')&& (type[6] == 't')){
                vecOfFiles.push_back(ch);
            }
        }
        closedir (dir);
    }
    path_to_return += vecOfFiles[(rand() % vecOfFiles.size())];
    return path_to_return;
}

bool ifWin(map<vector<int>,char>& mapOfWinPositions){
    decltype(mapOfWinPositions.size()) winCnt = 0;
    for (auto cross : mapOfWinPositions){
        if (cross.second == 'B'){
            winCnt++;
        }
    }
    if (winCnt == mapOfWinPositions.size()) {
        return true;
    }
    else{
        return false;
    }
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void ConstructAndSendBuffer(const int socket_desctriptor, TwoDimArray<char>& TDA) {
    string tobuff = std::to_string(TDA.getDimY()) + " " + std::to_string(TDA.getDimX());
    char newb[TDA.getDimX() * TDA.getDimY() + tobuff.size() + TDA.getDimY()];
    for (int i = 0; i < tobuff.size(); i++) {
        newb[i] = tobuff[i];
    }
    for (int i = tobuff.size(), j = 0;
         i <= TDA.getDimX() * TDA.getDimY() + tobuff.size() + TDA.getDimY();) {
        if (((i - tobuff.size()) % ((TDA.getDimX()) + 1) == 0)) {
            newb[i] = '\n';
        } else {
            newb[i] = TDA.getArray()[j];
            j++;
        }
        if (i == TDA.getDimX() * TDA.getDimY() + tobuff.size() + TDA.getDimY()) {
            newb[i] = '\0';
        }
        i++;
    }
    if (send(socket_desctriptor, newb, TDA.getDimX() * TDA.getDimY() + tobuff.size() + TDA.getDimY(), 0) ==
        -1)
        perror("default send to 1-st client");
}
