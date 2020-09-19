{% if pillar['cluster'][grains['id']]['is_primary'] %}
Stage - Init server:
  cmd.run:
    - name: __slot__:salt:setup_conf.conf_cmd('/opt/seagate/cortx/nfs/conf/setup.yaml', 'nfs_server:init')
Stage - Config server:
  cmd.run:
    - name: __slot__:salt:setup_conf.conf_cmd('/opt/seagate/cortx/nfs/conf/setup.yaml', 'nfs_server:config')
Stage - Restart server:
  cmd.run:
    - name: __slot__:salt:setup_conf.conf_cmd('/opt/seagate/cortx/nfs/conf/setup.yaml', 'nfs_server:restart')
{% else %}
Stage - Config server:
  cmd.run:
    - name: __slot__:salt:setup_conf.conf_cmd('/opt/seagate/cortx/nfs/conf/setup.yaml', 'nfs_server:config')
{% endif %}
