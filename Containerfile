FROM fedora:35

RUN dnf install -y \
    git g++ make flex bison gdb vim diffutils \
    bash-completion && dnf clean all
