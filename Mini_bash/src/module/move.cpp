#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <cerrno>
#include <string>
#include "../../include/module/move.hpp"

move::move(std::string __old_path, std::string __new_path, int __dflag = 0)
    : old_path(__old_path), new_path(__new_path), dflag(__dflag) {
        idtig = (dflag & IDTIG) == IDTIG;
    }

void move::__MoveCrossDevice(){
            struct stat meta_target;
            char *buff;
            int new_path_fd;
            int old_path_fd;
            ssize_t readed_bytes;
            ssize_t wroten_bytes;

            if((lstat(new_path.c_str(), &meta_target) == 0)){
                if(!((dflag & DFOVR) == DFOVR)){
                    char ans;
                    std::cout << "Overwrite '" << new_path << "' ?\n(y to continue) ";
                    std::cin >> ans;
                    if(ans != 'y') { return; }
                }                
            } else {
                std::cerr << "[Move] Fetching metadata : " << strerror(errno) << std::endl;
                throw "Fetching metadata failed";
            }
            
            if((new_path_fd = open(new_path.c_str(), O_RDWR | O_CREAT)) == -1){
                std::cerr << "[Move] Open new path : " << strerror(errno) << std::endl;
                throw "Open file failed";
            }
            if((old_path_fd = open(old_path.c_str(), O_RDONLY)) == -1){
                std::cerr << "[Move] Open old path : " << strerror(errno) << std::endl;
                throw "Open file failed";
            }
            buff = new char[meta_target.st_blksize];
            std::cout << "Buffer allocated !" << std::endl;
            try {
                while((readed_bytes = read(old_path_fd, buff, meta_target.st_blksize)) > 0){
                    wroten_bytes = write(new_path_fd, buff, readed_bytes);
                    if(wroten_bytes == -1){
                        std::cerr << "[Move] write data : " << strerror(errno) << std::endl;
                        throw "Writing data failed";
                    }

                    if((wroten_bytes != readed_bytes) && !idtig){
                        char cont;
                        std::cout << "Readed bytes : " << readed_bytes << "\nWroten bytes : " << wroten_bytes << "\nsize : " << meta_target.st_blksize << std::endl ;
                        std::cout << "stop/ignore ? (s/i) ";
                        std::cin >> cont;
                        if(cont == 's'){
                            throw "Process aborted by user";
                        } else if (cont == 'i'){
                            idtig = true;
                        }
                    }
                }
                if(readed_bytes == -1){ throw "Read data failed"; }
                
            } catch (const char* err_msg){
                std::cout << err_msg << std::endl;
                goto failcase_cleanup;
            }

            std::cout << "Success case cleanup !" << std::endl;
            close(new_path_fd);
            close(old_path_fd);
            remove(old_path.c_str());
            goto exit_jump;    
            
            failcase_cleanup:
            std::cout << "Failcase cleanup" << std::endl;
            close(new_path_fd);
            close(old_path_fd);
            remove(new_path.c_str());

            exit_jump:
            delete[] buff;
            std::cout << "Buffer freed !" << std::endl;
            return;
        }

void move::__Move(){
    struct stat tmpbuff;
    if((lstat(new_path.c_str(), &tmpbuff) == 0)){
        if(!((dflag & DFOVR) == DFOVR)){
            char ans;
            std::cout << "Overwrite '" << new_path << "' ?\n(y to continue) ";
            std::cin >> ans;
            if(ans != 'y') { return; }
        }
    }
    if(rename(old_path.c_str(), new_path.c_str()) == -1){
        std::cerr << "[Move] renaming file : "  << std::endl;
        throw "File renaming failed";
    };
}

void move::feature_main(){
    if((dflag & DCDEV) == DCDEV){
        __MoveCrossDevice();
    } else {
        __Move();
    }
}

int main(){
    move Move("/home/adis/Downloads/Games/data.csv", "/home/adis/Downloads/Games/ADCS-API/data.csv", DCDEV);    
    Move.feature_main();
    
    return 0;


}
