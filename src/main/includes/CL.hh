// Copyright (C) 2013 Dominik Dahlem <Dominik.Dahlem@gmail.com>
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

/** @file CL.hh
 * Declaration of the command line parameters for disease network generation.
 *
 * @author Dominik Dahlem
 */
#ifndef __CANOPY_MAIN_CL_HH__
#define __CANOPY_MAIN_CL_HH__

#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif /* __STDC_CONSTANT_MACROS */

#include <boost/cstdint.hpp>

#include <boost/program_options/options_description.hpp>
namespace po = boost::program_options;

#include <boost/scoped_ptr.hpp>


namespace canopy
{
namespace main
{

/**
 * const variables specifying the allowed options.
 */
const std::string HELP = "help";
const std::string VERS = "version";

const std::string SEQUENCE = "sequence";
const std::string RESULTS_DIR = "result";
const std::string T1 = "t1";
const std::string T2 = "t2";
const std::string SAMPLE = "sample";
const std::string PAIRS = "pairs";


/** @struct
 * structure specifying the command line variables.
 */
struct args_t {
    std::string sequence;              /* input matrix in CSV */
    std::string results_dir;        /* directory name for the results */
    std::string pairs;        // pairs to calculate distances for
    double t1;
    double t2;
    boost::uint32_t sample;

    args_t(args_t const &args)
        : sequence(args.sequence), results_dir(args.results_dir), pairs(args.pairs), t1(args.t1), t2(args.t2),
          sample(args.sample)
        {}

    args_t()
        : sequence(""), results_dir(""), pairs(""), t1(0.0), t2(0.0), sample(0)
        {}

    friend std::ostream& operator <<(std::ostream &p_os, const args_t &p_args)
        {
            p_os << "Parameters" << std::endl << std::endl;
            p_os << "Sequence file:     " << p_args.sequence << std::endl
                 << "Results directory: " << p_args.results_dir << std::endl
                 << "Pairs:             " << p_args.pairs << std::endl
                 << "T1:                " << p_args.t1 << std::endl
                 << "T2:                " << p_args.t2 << std::endl
                 << "Sample:            " << p_args.sample << std::endl
                 << std::endl;

            return p_os;
        }
};



/** @class CL
 * This class uses the boost program-options library to parse the command-line
 * parameters for the main routine of the discrete event simulator.
 */
class CL
{
public:
    CL();

    /** @fn parse(int argc, char *argv[], args_t);
     * Parse the command-line parameters and store the relevant information
     * in a shared pointer of a structure.
     *
     * @param int number of command-line arguments
     * @param char** the command-line arguments
     * @param args_t a reference to the structure of the command-line
     *        arguments
     * @return either success or failure. In case of a failure then the help
     *        message was requested.
     */
    int parse(int, char **, args_t &);

private:

    /**
     * A scoped pointer to the description of the command-line arguments.
     */
    boost::scoped_ptr<po::options_description> m_opt_desc;
};


    }
}


#endif
