// this is a silly solution
// just to show you how different
// components of this framework work
// please bring your wise to write
// a 'real' solution :)

#include <iostream>
#include <fstream>
#include <string>
#include "../../include/IR.h"
#include "../../3rdparty/jsoncpp/include/json/json.h"
#include "../../include/mytest.h"
#include "../../include/y.tab.h"
#include "../../include/IRPrinter_genCcode.h"
#include <io.h>

extern std::map<std::string, std::pair<int, int>> global_map;
extern std::map<std::string, std::vector<size_t>> global_shape_map;

void handler(std::string inpath, std::string outpath)
{
    global_map.clear();
    global_shape_map.clear();
    Boost::Internal::Ref<const Boost::Internal::IRNode> myroot;

    Json::CharReaderBuilder reader_builder;
    Json::Value root;
    JSONCPP_STRING errs;
    std::ofstream ofile(outpath, std::ios::out);
    std::ifstream infile(inpath, std::ios::binary);
    if (!infile.is_open())
    {
        std::cout << inpath << " open file failed\n";
        return;
    }
    bool parse_ok = Json::parseFromStream(reader_builder, infile, &root, &errs);
    if (!parse_ok)
    {
        std::cout << inpath << " parse failed\n";
        return;
    }
    std::string str = root["kernel"].asString();
    myroot = yyparse_string((char *)(str.c_str()));
    std::shared_ptr<Boost::Internal::Kernel> myroot_kernel = std::const_pointer_cast<Boost::Internal::Kernel>(std::dynamic_pointer_cast<const Boost::Internal::Kernel>(myroot.real_ptr()));
    std::set<std::string> array_recorded;
    for (auto item : root["ins"])
    {
        std::string varname = item.asString();
        if (array_recorded.find(varname) != array_recorded.end())
            continue;
        array_recorded.insert(varname);
        std::shared_ptr<Boost::Internal::Var> ptr = std::make_shared<Boost::Internal::Var>(Boost::Internal::Type::float_scalar(32), "(&" + varname + ")", std::vector<Boost::Internal::Expr>(), global_shape_map[varname]);
        for (auto dim : ptr->shape)
        {
            ptr->args.push_back(int(dim));
        }
        myroot_kernel->inputs.push_back(std::const_pointer_cast<const Boost::Internal::Var>(ptr));
    }
    for (auto item : root["outs"])
    {
        std::string varname = item.asString();
        if (array_recorded.find(varname) != array_recorded.end())
            continue;
        array_recorded.insert(varname);
        std::shared_ptr<Boost::Internal::Var> ptr = std::make_shared<Boost::Internal::Var>(Boost::Internal::Type::float_scalar(32), "(&" + varname + ")", std::vector<Boost::Internal::Expr>(), global_shape_map[varname]);
        for (auto dim : ptr->shape)
        {
            ptr->args.push_back(int(dim));
        }
        myroot_kernel->outputs.push_back(std::const_pointer_cast<const Boost::Internal::Var>(ptr));
    }
    myroot_kernel->printer_data_type = root["data_type"].asString();
    myroot_kernel->name = root["name"].asString();
    Boost::Internal::IRPrinter_genCcode printer;
    ofile << printer.print(Boost::Internal::Group(myroot_kernel));

    ofile.close();
    std::cout << inpath << " done, saved at " << outpath << "\n";
}

int main()
{
    std::string InDir = "./cases/*.json";
    std::string OutDir = "./kernels/";
    //用于查找的句柄
    long handle;
    struct _finddata_t fileinfo;
    //第一次查找
    handle = _findfirst(InDir.c_str(), &fileinfo);
    if (handle == -1)
        return -1;
    do
    {
        std::string filename(fileinfo.name);
        std::string OutPath = OutDir + "kernel_" + filename.substr(0, filename.length() - 5) + ".cc";

        handler("./cases/" + filename, OutPath);

    } while (!_findnext(handle, &fileinfo));

    _findclose(handle);

    return 0;
}