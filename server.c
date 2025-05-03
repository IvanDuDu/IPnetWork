
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
int main() {
fd_set fdset;
FD_ZERO(&fdset);
int fd[24]={0};

while(1){
    int k=-1;
    scanf("Nhap cac cong muon them : %d", &k);
    if(fd[k]){
        FD_SET(k, &fdset);
        fd[k]=1; 

    }else{
        printf("Da tao tu truoc");
    }

}
printf("fds_bits[0] = %ld\n", fdset.__fds_bits[0]);
// Hiển thị giá trị nhị phân của fds_bits[0]
for (int i = sizeof(fdset.__fds_bits[0])*8 - 1; i >= 0; i--) {
printf("%ld", (fdset.__fds_bits[0] >> i) & 1);
}
printf("\n");
return 0;
}
