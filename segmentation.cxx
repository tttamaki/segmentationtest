//
//  Created by Toru Tamaki on 2014/2/11.
//  Copyright (c) 2014 tamaki. All rights reserved.
//

//
// code taken from
// - opengm/src/examples/image-processing-examples/grid_potts.cxx at http://hci.iwr.uni-heidelberg.de/opengm2/
//



#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <Eigen/Dense>
#include <Eigen/Core>
#include <opencv2/core/eigen.hpp>

#include <opengm/graphicalmodel/graphicalmodel.hxx>
#include <opengm/graphicalmodel/space/simplediscretespace.hxx>
#include <opengm/operations/adder.hxx>
#include <opengm/operations/minimizer.hxx>
#include <opengm/functions/potts.hxx>
#include <opengm/inference/graphcut.hxx>
#include <opengm/inference/alphabetaswap.hxx>
#include <opengm/inference/alphaexpansion.hxx>
#include <opengm/inference/auxiliary/minstcutkolmogorov.hxx>






#include "readCSV.h"



#include "boost/program_options.hpp"

struct options {
    std::string filename;
    double p;

    std::string outfilename;
    std::string outimage;
};

options parseOptions(int argc, char* argv[]) {
    
    options Opt;
    
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
    ("help", "This help message.")
    ("file", po::value<std::string>(), "data filename (csv)")
    ("p", po::value<double>()->default_value(0.9), "parameter p: default 0.9")
    ("outfile", po::value<std::string>()->default_value(""), "out put data filename")
    ("outimg", po::value<std::string>()->default_value(""), "out put image filename")
    ;
    
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        exit(0);
    }
    
    if (vm.count("file"))
        Opt.filename = vm["file"].as<std::string>();
    else{
        std::cout << "no filename is specified." << std::endl
        << desc << std::endl;
        exit(1);
    }

    if (vm.count("p"))
        Opt.p = vm["p"].as<double>();


    if (vm.count("outfile"))
        Opt.outfilename = vm["outfile"].as<std::string>();
    if (vm.count("outimg"))
        Opt.outimage = vm["outimg"].as<std::string>();

    
    
    
    
    
    
    return Opt;
}









void
imshow(const std::string &title,
       const std::vector<Eigen::MatrixXf> &gridvec,
       const std::string &outimage)
{
    
    cv::Mat _rgb[3], rgb;
    cv::eigen2cv(gridvec[0], _rgb[0]);
    cv::eigen2cv(gridvec[1], _rgb[1]);
    cv::eigen2cv(gridvec[2], _rgb[2]);
    
    
    cv::merge(_rgb, 3, rgb);

    
    if (! outimage.empty() ) {
        cv::imwrite(outimage, rgb*255.0);
    } else {
        cv::imshow(title, rgb);
    }

}







inline size_t variableIndex(const size_t x, const size_t y, const size_t nx) {
    return x + nx * y;
}





int main(int argc, char* argv[]) {
    
    options Opt = parseOptions(argc, argv);

    
    
    
    //
    // read data from file
    //
    std::vector< std::vector< double > > data;
    data.reserve(1000);
    
    readFile(Opt.filename, data);
    
    data.shrink_to_fit();
    size_t dataSize = data.size();
    //
    //
    //


    // due to the special format of input files
    size_t nx = data[0].size();
    size_t ny = data.size() / 3;
    
    
    
    size_t numberOfLabels = 3; // 3 classes

    
    
    //
    // construct graphical model
    //
    typedef opengm::DiscreteSpace<size_t, size_t> Space;
    Space space((size_t)(nx * ny), numberOfLabels);
    
    typedef opengm::GraphicalModel<double, opengm::Adder,
                                    OPENGM_TYPELIST_2(opengm::ExplicitFunction<double>,
                                                      opengm::PottsFunction<double>),
                                    Space> Model;
    Model gm(space);
    

    //
    // set factors
    //
    for(size_t y = 0; y < ny; ++y) {
        for(size_t x = 0; x < nx; ++x) {
            // function
            const size_t shape[] = {numberOfLabels};
            opengm::ExplicitFunction<double> f(shape, shape + 1);
            
            for(size_t s = 0; s < numberOfLabels; ++s) {
                double tmp = data[y*3 + s][x]; // due to the special format of input files
                f(s) = -log(tmp);
            }
            Model::FunctionIdentifier fid = gm.addFunction(f);
            
            // factor
            size_t variableIndices[] = {variableIndex(x, y, nx)};
            gm.addFactor(fid, variableIndices, variableIndices + 1);
        }
    }
    
    
    // pairwise
    double same = -std::log(Opt.p);
    double diff = -std::log( (1.0-Opt.p) / 2.0);
    
    opengm::PottsFunction<double> f2(numberOfLabels, numberOfLabels,
                                     same, // same labels
                                     diff); // different labels
    typename Model::FunctionIdentifier fid2 = gm.addFunction(f2);
    
    for(size_t y = 0; y < ny; ++y)
        for(size_t x = 0; x < nx; ++x) {
            if(x + 1 < nx) { // (x, y) -- (x + 1, y)
                size_t variableIndices[] = {variableIndex(x, y, nx), variableIndex(x + 1, y, nx)};
                std::sort(variableIndices, variableIndices + 2);
                gm.addFactor(fid2, variableIndices, variableIndices + 2);
            }
            if(y + 1 < ny) { // (x, y) -- (x, y + 1)
                size_t variableIndices[] = {variableIndex(x, y, nx), variableIndex(x, y + 1, nx)};
                std::sort(variableIndices, variableIndices + 2);
                gm.addFactor(fid2, variableIndices, variableIndices + 2);
            }
        }
    
    

    
    std::vector<size_t> labeling(nx * ny);

    //
    // inference by a-b swap graph cut
    //
    typedef opengm::external::MinSTCutKolmogorov<size_t, double> MinStCutType;
    typedef opengm::GraphCut<Model, opengm::Minimizer, MinStCutType> MinGraphCut;
    typedef opengm::AlphaBetaSwap<Model, MinGraphCut> MinAlphaBetaSwap;
    MinAlphaBetaSwap abs(gm);
    MinAlphaBetaSwap::VerboseVisitorType visitor;
    abs.infer(visitor);
    abs.arg(labeling); // obtain the result
    
   
    
    //
    // output the (approximate) argmin
    //
    {
        std::vector<Eigen::MatrixXf> gridvec2;
        for(size_t s = 0; s < numberOfLabels; ++s) {
            Eigen::MatrixXf mygrid = Eigen::MatrixXf::Zero(ny,nx);
            gridvec2.push_back(mygrid);
        }

        size_t variableIndex = 0;
        for(size_t y = 0; y < ny; ++y) {
            for(size_t x = 0; x < nx; ++x) {
                gridvec2[labeling[variableIndex++]](y,x) = 1;
            }
        }
        
        imshow("RGB2", gridvec2, Opt.outimage);
        if (Opt.outfilename.empty() ) {
            cv::waitKey(0);
        }
        
    }

    
    if (! Opt.outfilename.empty() ) {
        std::ofstream ofs(Opt.outfilename);
        size_t variableIndex = 0;

        for(size_t y = 0; y < ny; ++y) {
            for(size_t x = 0; x < nx; ++x) {
                ofs << labeling[variableIndex++];
            }
            ofs << std::endl;
        }
        ofs.close();
    }

    

    
    return 0;
}
