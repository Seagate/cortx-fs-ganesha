Install prereq packages for NFS:
  pkg.installed:
    - pkgs:
      - jemalloc
      - krb5-devel
      - libini_config-devel
      - krb5-server
      - krb5-libs
      - nfs-utils
      - rpcbind
      - libblkid
      - libevhtp

Install NFS Ganesha:
  pkg.installed:
    - name: nfs-ganesha

Install NFS packages:
  pkg.installed:
    - pkgs:
      - cortx-utils
      - cortx-utils-devel
      - cortx-nsal
      - cortx-nsal-devel
      - cortx-dsal
      - cortx-dsal-devel
      - cortx-fs
      - cortx-fs-devel
      - cortx-fs-ganesha
      - cortx-fs-ganesha-test
    - disableexcludes: main
