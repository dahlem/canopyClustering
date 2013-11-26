// Copyright (C) 2013 Dominik Dahlem <dahlem@mit.edu>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif /* __STDC_CONSTANT_MACROS */

#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/cstdint.hpp>
#include <boost/flyweight.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <boost/tuple/tuple.hpp>

#include "CL.hh"
namespace cmain = canopy::main;

#include "NCD.hh"
namespace ncd = canopy::ncd;

typedef boost::flyweight<boost::uint32_t> fw_uint32;
typedef boost::unordered_set<fw_uint32> set;
typedef std::pair<std::string, boost::uint32_t> val;
typedef boost::unordered_map<fw_uint32, val > map;


static fw_uint32::initializer fw_uint32_init;
boost::mt19937 gen;




int main(int argc, char *argv[])
{
    map m_map;

    std::ios_base::sync_with_stdio(false);
    cmain::args_t args;
    cmain::CL cl;

    if (cl.parse(argc, argv, args)) {
        return EXIT_SUCCESS;
    }

    std::ifstream sequenceFile;
    sequenceFile.open(args.sequence.c_str());
    if (!sequenceFile.is_open()) {
        std::cerr << "Could not open file: " << args.sequence << std::endl;
        return EXIT_FAILURE;
    }

    // parsing
    std::string line;
    while (!sequenceFile.eof()) {
        std::getline(sequenceFile, line);
        boost::trim(line);

        if (line != "") {
            std::vector<std::string> splitVec;
            boost::split(splitVec, line, boost::is_any_of(","), boost::token_compress_on);
            fw_uint32 seqID = fw_uint32(boost::lexical_cast<boost::uint32_t>(splitVec[0]));
            m_map[seqID] = std::pair<std::string, boost::uint32_t> (splitVec[1], 0);
        }
    }
    sequenceFile.close();

    if (args.pairs != "") {
        std::vector<std::pair<fw_uint32, fw_uint32> > pairs;

        std::ifstream pairsFile;
        pairsFile.open(args.pairs.c_str());
        if (!pairsFile.is_open()) {
            std::cerr << "Could not open file: " << args.pairs << std::endl;
            return EXIT_FAILURE;
        }
        while (!pairsFile.eof()) {
            std::getline(pairsFile, line);
            boost::trim(line);

            if (line != "") {
                std::vector<std::string> splitVec;
                boost::split(splitVec, line, boost::is_any_of(","), boost::token_compress_on);
                fw_uint32 seqID1 = fw_uint32(boost::lexical_cast<boost::uint32_t>(splitVec[0]));
                fw_uint32 seqID2 = fw_uint32(boost::lexical_cast<boost::uint32_t>(splitVec[1]));
                pairs.push_back(std::pair<fw_uint32, fw_uint32>(seqID1, seqID2));
            }
        }
        pairsFile.close();

        #pragma omp parallel for shared(m_map) default(none)
        for (boost::uint32_t i = 0; i < m_map.size(); ++i) {
            map::iterator h_i = m_map.begin();
            std::advance(h_i, i);
            #pragma omp critical
            {
                (h_i->second).second = ncd::compressStr((h_i->second).first.c_str(), (h_i->second).first.length());
#ifndef NDEBUG
                std::cout << h_i->first << "," << (h_i->second).first.length() << "," << (h_i->second).second << std::endl;
#endif
            }
        }
        #pragma omp parallel shared(pairs, m_map)
        {
            std::vector<boost::tuple<fw_uint32, fw_uint32, double> > dists;

            #pragma omp for
            for (boost::uint32_t i = 0; i < pairs.size()-1; ++i) {
                double dist = ncd::NCD<val, double>()(m_map[pairs[i].first], m_map[pairs[i].second]);
                dists.push_back(boost::tuple<fw_uint32, fw_uint32, double>(pairs[i].first, pairs[i].second, dist));
            }
            #pragma omp critical
            {
                for (std::vector<boost::tuple<fw_uint32, fw_uint32, double> >::iterator d_it = dists.begin(); d_it != dists.end(); ++d_it) {
                    std::cout << d_it->get<0>() << "," << d_it->get<1>() << "," << d_it->get<2>() << std::endl;
                }
            }
        }
    } else if (args.sample > 0) {
        boost::uniform_int<> dist(0, m_map.size() - 1);
        boost::variate_generator<boost::mt19937&, boost::uniform_int<> > rand(gen, dist);
        if (args.sample > m_map.size()) {
            args.sample = m_map.size();
        }
        std::vector<boost::uint32_t> sample;
        for (boost::uint32_t i = 0; i < m_map.size(); ++i) {
            sample.push_back(i);
        }
        std::random_shuffle(sample.begin(), sample.end(), rand);
        sample.resize(args.sample);

        #pragma omp parallel for shared(sample, m_map)
        for (boost::uint32_t i = 0; i < sample.size(); ++i) {
            map::iterator h_i = m_map.begin();
            std::advance(h_i, sample[i]);
            #pragma omp critical
            {
                (h_i->second).second = ncd::compressStr((h_i->second).first.c_str(), (h_i->second).first.length());
#ifndef NDEBUG
                std::cout << h_i->first << "," << (h_i->second).first.length() << "," << (h_i->second).second << std::endl;
#endif
            }
        }

#ifndef NDEBUG
            std::cout << "compute ncd distances..." << std::endl;
#endif

        #pragma omp parallel shared(sample, m_map)
        {
            std::vector<boost::tuple<fw_uint32, fw_uint32, double> > dists;

            #pragma omp for
            for (boost::uint32_t i = 0; i < sample.size()-1; ++i) {
                map::const_iterator h_i = m_map.begin();
                std::advance(h_i, sample[i]);
                for (boost::uint32_t j = i + 1; j < sample.size(); ++j) {
                    map::const_iterator h_j = m_map.begin();
                    std::advance(h_j, sample[j]);
                    double dist = ncd::NCD<val, double>()(h_i->second, h_j->second);
                    dists.push_back(boost::tuple<fw_uint32, fw_uint32, double>(h_i->first, h_j->first, dist));
                }
            }
            #pragma omp critical
            {
                for (std::vector<boost::tuple<fw_uint32, fw_uint32, double> >::iterator d_it = dists.begin(); d_it != dists.end(); ++d_it) {
                    std::cout << d_it->get<0>() << "," << d_it->get<1>() << "," << d_it->get<2>() << std::endl;
                }
            }
        }
    } else {
        // canopies
        boost::uint32_t c = 0;
        boost::unordered_map<fw_uint32, set> canopies;

        #pragma omp parallel for shared(m_map) default(none)
        for (boost::uint32_t i = 0; i < m_map.size(); ++i) {
            map::iterator h_i = m_map.begin();
            std::advance(h_i, i);
            #pragma omp critical
            {
                (h_i->second).second = ncd::compressStr((h_i->second).first.c_str(), (h_i->second).first.length());
#ifndef NDEBUG
                std::cout << h_i->first << "," << (h_i->second).first.length() << "," << (h_i->second).second << std::endl;
#endif
            }
        }

        while (m_map.size() > 0) {
            boost::uniform_int<> dist(0, m_map.size() - 1);
            boost::variate_generator<boost::mt19937&, boost::uniform_int<> > rand(gen, dist);
            map::const_iterator item = m_map.begin();
            std::advance(item, rand());
            std::vector<fw_uint32> canopyElements;

            // put random element into canopy
            set inCanopy;
            inCanopy.insert(item->first);
            canopies[fw_uint32(c)] = inCanopy;
            canopyElements.push_back(item->first);

            #pragma omp parallel for shared(m_map, canopyElements, c, args, std::cout, canopies, item) default(none)
            for (boost::uint32_t i = 0; i < m_map.size(); ++i) {
                map::const_iterator h_it = m_map.begin();
                std::advance(h_it, i);
                if (h_it != item) {
                    double dist = ncd::NCD<val, double>()(h_it->second, item->second);
                    #pragma omp critical
                    {
#ifndef NDEBUG
                        std::cout << h_it->first << "," << item->first << "," << dist << std::endl;
#endif
                        if (dist < args.t1) {
                            canopies[fw_uint32(c)].insert(h_it->first);
                        }
                        if (dist < args.t2) {
                            canopyElements.push_back(h_it->first);
                        }
                    }
                }
           }
           for (std::vector<fw_uint32>::iterator t2_it = canopyElements.begin(); t2_it != canopyElements.end(); ++t2_it) {
               map::iterator h_it = m_map.find(*t2_it);
               if (h_it != m_map.end()) {
                   m_map.erase(h_it);
               }
           }
           c++;
        }
        for (boost::unordered_map<fw_uint32, set>::const_iterator c_it = canopies.begin(); c_it != canopies.end(); ++c_it) {
            for (set::const_iterator s_it = (c_it->second).begin(); s_it != (c_it->second).end(); ++s_it) {
                std::cout << c_it->first << "," << *s_it << std::endl;
            }
        }
    }

    return EXIT_SUCCESS;
}
