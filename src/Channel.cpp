#include"Channel.h"

Channel::Channel(int fd):fd_(fd),index_(-1),events_(0),revents_(0){

}

void Channel::handleEvent(){
    if((revents_ & EPOLLERR) || (revents_ & EPOLLHUP)){
        if(errorCallback_){
            errorCallback_();
        }
    }

    if(revents_ & EPOLLIN){
        if(readCallback_){
            readCallback_();
        }
    }

    if(revents_ & EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
}