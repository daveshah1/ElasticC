echo "#include \"Util.hpp\"" > src/version.cpp
echo "const string ElasticC::ecc_version = \"0.3-git-`git rev-parse --short HEAD`\";" >> src/version.cpp
