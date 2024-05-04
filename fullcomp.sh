mkdir builds
python3 ./scripts/generate_limine_cfg.py

cd ./scripts/gen2_registry/
bash gen2_registry_compile.sh
mv ./gen2_reg ../../gen2_reg
cd ../../

./gen2_reg
mv sys_registry files/registry
make run -j6
# python3 ./scripts/finish.py
