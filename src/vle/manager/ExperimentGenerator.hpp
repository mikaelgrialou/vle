/**
 * @file src/vle/manager/ExperimentGenerator.hpp
 * @author The VLE Development Team
 */

/*
 * VLE Environment - the multimodeling and simulation environment
 * This file is a part of the VLE environment (http://vle.univ-littoral.fr)
 * Copyright (C) 2003 - 2008 The VLE Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef VLE_MANAGER_EXPERIMENTGENERATOR_HPP
#define VLE_MANAGER_EXPERIMENTGENERATOR_HPP

#include <vle/value/Value.hpp>
#include <vle/vpz/Vpz.hpp>
#include <vle/manager/Types.hpp>
#include <string>
#include <vector>
#include <list>
#include <glibmm/thread.h>



namespace vle { namespace manager {

    /** 
     * @brief A class to translate Experiment file into Instance of Experiment.
     */
    class ExperimentGenerator
    {
    public:
        /**
         * Just get a constant reference to VPZ. Use get_instances_files() to
         * generate all VPZ instance file.
         *
         */
        ExperimentGenerator(const vpz::Vpz& file, std::ostream& out);

        virtual ~ExperimentGenerator() { }

        /**
         * @brief Get the list of filename build after build.
         * @return the list of filename.
         */
        std::list < vpz::Vpz* >& vpzInstances()
        { return mFileList; }

        void build(OutputSimulationMatrix* matrix);

        void build(Glib::Mutex* mutex, Glib::Cond* prod,
                   OutputSimulationMatrix* matrix);

        /** 
         * @brief Save the generated VPZ instance file.
         * 
         * @param save true to save the files, otherwise, this file are remove.
         * The default option is to remove file.
         */
        void saveVPZinstance(bool save = false)
        { mSaveVpz = save; }

    protected:
        /** 
         * @brief Build just one condition based on information of VPZ file.
         */
        virtual void buildCombination(size_t& nb) = 0;

        /** 
         * @brief Get the number of combination from vpz file.
         * @return A value greater than 0.
         */
        virtual size_t getCombinationNumber() const = 0;
        
        /*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

        /** 
         * @brief Build a list of replicas based on information of VPZ file.
         */
        void buildReplicasList();

        /** 
         * @brief Build a list of conditions based on information of VPZ file.
         */
        void buildConditionsList();

        void buildCombinations();

        void buildCombinationsFromReplicas(size_t cmbnumber);

        /** 
         * @brief Build an instance of the current specified vpz file and push
         * it into the output list. This function use the Glib::Mutex and
         * Glib::Cond to protect the access to data.
         * @param cmbnumber the index of the combination.
         * @param replnumber the number of the replicas.
         */
        void writeInstanceThread(size_t cmbnumber, size_t replnumber);
        
        /** 
         * @brief Build an instance of the current specified vpz file and push
         * it into the output list.
         * @param cmbnumber the index of the combination.
         * @param replnumber the number of the replicas.
         */
        void writeInstance(size_t cmbnumber, size_t replnumber);

        /** 
         * @brief Get the number of replicas from vpz file.
         * @return A value greater than 0.
         */
        size_t getReplicasNumber() const;

        /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    protected:
        /** 
         * @brief Define a condition used to generate combination list.
         */
        struct cond_t {
            cond_t() : sz(0), pos(0) { }

            size_t  sz;
            size_t  pos;
        };

        void cleanCondition(vpz::Vpz& file);

        const vpz::Vpz&             mFile;
        vpz::Vpz                    mTmpfile;
        std::ostream&               mOut;
        std::vector < guint32 >     mReplicasTab;
        std::vector < cond_t >      mCondition;
        std::list < vpz::Vpz* >     mFileList;
        bool                        mSaveVpz;
        OutputSimulationMatrix*     mMatrix;
        Glib::Mutex*                mMutex;
        Glib::Cond*                 mProdcond;
    };

}} // namespace vle manager

#endif
