//
//  readCSV.cxx
//  hmm
//
//  Created by tamaki on 2014/02/03.
//  Copyright (c) 2014 tamaki. All rights reserved.
//

#include "readCSV.h"


void
readFile(const std::string &filename,
         std::vector< std::vector< double > > &data)
{
    
    std::ifstream ifs(filename.c_str());
    if (ifs.is_open()) {
        // std::cout << "read file: " << filename << std::endl;
    } else {
        std::cerr << "Fail to read:: " << filename << std::endl;
        exit(1);
    }
    
    typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
    
    std::vector< std::string > vecs;
    std::vector< double > vecf;
    std::string line;
    
    boost::char_separator<char> sep(" \t"); // separated by spaces and tabs
    
    size_t count = 0;
    while (std::getline(ifs,line))
    {
        count++;
        if (count % 4 == 0) continue; // skip every fourth lines

        Tokenizer tok(line, sep);
        vecs.assign(tok.begin(),tok.end());
        
        vecf.resize(vecs.size());
        for (size_t i = 0; i < vecs.size(); i++) {
            vecf[i] = std::stof(vecs[i]);
        }
        
        data.push_back(vecf);
        
        
    }
}