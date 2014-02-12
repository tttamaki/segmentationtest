//
//  readCSV.h
//  hmm
//
//  Created by tamaki on 2014/02/03.
//  Copyright (c) 2014 tamaki. All rights reserved.
//

#ifndef __hmm__readCSV__
#define __hmm__readCSV__

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

#include <boost/tokenizer.hpp>


void
readFile(const std::string &filename,
         std::vector< std::vector< double > > &data);

#endif /* defined(__hmm__readCSV__) */
