echo "#include \"Util.hpp\"" > src/version.cpp
echo "const string RapidHLS::rhls_version = \"0.3-git-`git rev-parse --short HEAD`\";" >> src/version.cpp
