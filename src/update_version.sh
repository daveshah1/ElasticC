echo "#include \"Util.hpp\"" > src/version.cpp
echo "const string ElasticC::ecc_version = \"`git describe`-git`git diff-index --quiet HEAD -- || echo -dirty`\";" >> src/version.cpp
