/*
 * @file vle/utils/RemoteManager.cpp
 *
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems
 * http://www.vle-project.org
 *
 * Copyright (c) 2003-2007 Gauthier Quesnel <quesnel@users.sourceforge.net>
 * Copyright (c) 2003-2010 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2010 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and contributors
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


#include <vle/utils/RemoteManager.hpp>
#include <vle/utils/Algo.hpp>
#include <vle/utils/DownloadManager.hpp>
#include <vle/utils/Exception.hpp>
#include <vle/utils/Path.hpp>
#include <vle/utils/Package.hpp>
#include <vle/utils/Preferences.hpp>
#include <vle/utils/Trace.hpp>
#include <vle/version.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <ostream>
#include <string>

namespace vle { namespace utils {

static bool isSource(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Source:");
}

static bool isVersion(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Version:");
}

static bool isSection(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Section:");
}

static bool isMaintainer(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Maintainer:");
}

static bool isHomepage(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Homepage:");
}

static bool isUrl(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Url:");
}

static bool isBuildDepends(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Build-Depends:");
}

static bool isDepends(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Depends:");
}

static bool isConflicts(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Conflicts:");
}

static bool isDescription(const std::string& line)
{
    return boost::algorithm::starts_with(line, "Description:");
}

static bool isEndDescription(const std::string& line)
{
    return line == " .";
}

/**
 * \c PackageVersion identifier of either a package or vle version
 *
 * the version of a package is :
 *  - the major version number
 *  - the minor version number
 *  - the patch number
 *  - the reopsitory number (for a package) or TODO (for vle)
 */

struct PackageVersion
{
    PackageVersion():
        mhasVersionMinor(false), mhasVersionPatch(false),
        mhasVersionRepository(false), mVersionMajor(0), mVersionMinor(0),
        mVersionPatch(0), mVersionRepository(0)
    {

    }

    /**
     * Fills version
     *
     * @param version_id, string representation of the version,
     *  with format "10", "10.2", "10.2.3" or "10.2.3-4"
     **/
    void fill(const std::string& version_id)
    {
        using boost::lexical_cast;
        using boost::numeric_cast;

        std::vector<std::string> splitRes;
        boost::algorithm::split(splitRes, version_id,
                                boost::algorithm::is_any_of("-"),
                                boost::algorithm::token_compress_on);

        if (splitRes.size() != 1 && splitRes.size() != 2){
            throw utils::ArgError(fmt(_("PackageVersion: bad format '%1%'"))
                    % version_id);
        }
        if (splitRes.size() == 2) {
            mVersionRepository = numeric_cast < uint32_t >(
                    lexical_cast < long >(splitRes[1]));
            mhasVersionRepository = true;
        } else {
            mVersionRepository = 0;
            mhasVersionRepository = false;
        }

        const std::string& majMinPa = splitRes[0];

        std::vector<std::string> splitRes2;
        boost::algorithm::split(splitRes2, majMinPa,
                                boost::algorithm::is_any_of("."),
                                boost::algorithm::token_compress_on);

        if (splitRes2.size() == 0 ){
            throw utils::ArgError(fmt(_("PackageVersion: bad format '%1%'"))
                    % version_id);
        }

        mVersionMajor = numeric_cast < uint32_t >(
                lexical_cast < long >(splitRes2[0]));

        if (splitRes2.size() > 1 ){
            mVersionMinor = numeric_cast < uint32_t >(
                            lexical_cast < long >(splitRes2[1]));
            mhasVersionMinor = true;
            if (splitRes2.size() > 2 ){
                mVersionPatch = numeric_cast < uint32_t >(
                        lexical_cast < long >(splitRes2[2]));
                mhasVersionPatch = true;
                if(splitRes2.size() > 3 ){
                    throw utils::ArgError(fmt(_("PackageVersion: bad format "
                            "'%1%'")) % version_id);
                }
            } else {
                mhasVersionPatch = false;
            }
        } else {
            mhasVersionMinor = false;
        }
    }
    bool mhasVersionMinor;
    bool mhasVersionPatch;
    bool mhasVersionRepository;
    uint32_t mVersionMajor;
    uint32_t mVersionMinor;
    uint32_t mVersionPatch;
    uint32_t mVersionRepository;
};

/**
 * \c PackageDepend a dependance to a package or vle.
 *
 * A dependance consists in :
 *  - the name of the package dependance
 *  - the version targeted
 *  - a boolean indicating if there should be a compatibilty with
 *  newer versions
 */
struct PackageDepend
{
    PackageDepend() :
        mName(""),mVersion(), mGreaterAccepted(false)
    {

    }
    /**
     * Fills dependance
     *
     * @param dep, string dependance representation eg. "weather (>= 1.2.3)"
     **/
    void fill(const std::string& dep)
    {
        std::vector<std::string> splitRes;
        boost::algorithm::split(splitRes, dep,
                                boost::algorithm::is_any_of(" "),
                                boost::algorithm::token_compress_on);

        if (splitRes.size() != 3) {
            throw utils::ArgError(fmt(_("PackageDepend: bad format '%1%'"))
                    % dep);
        } else {
            const std::string& name = splitRes[0];
            if (name.size() < 1){
                throw utils::ArgError(fmt(_("PackageDepend: bad name format "
                                        "'%1%'")) % dep);
            }
            mName.assign(name);
            const std::string& greater = splitRes[1];
            if (greater == "(>="){
                mGreaterAccepted = true;
            } else if (greater == "(=") {
                mGreaterAccepted = false;
            } else {
                throw utils::ArgError(fmt(_("PackageDepend: bad comparison "
                        "format '%1%'")) % dep);
            }
            const std::string& version = splitRes[2];
            if (version.size() < 2){
                throw utils::ArgError(fmt(_("PackageDepend: bad version "
                                        "format '%1%'")) % dep);
            }
            mVersion.fill(std::string(version,0,version.size()-1));
        }
    }

    std::string mName;
    PackageVersion mVersion;
    bool mGreaterAccepted;

};

std::ostream& operator<<(std::ostream& os, const PackageDepend& pd)
{
    os << pd.mName;
    if (pd.mGreaterAccepted){
        os << " (>= ";
    } else {
        os << " (= ";
    }
    os << pd.mVersion.mVersionMajor ;
    if (pd.mVersion.mhasVersionMinor) {
        os << "." << pd.mVersion.mVersionMinor;
    }
    if (pd.mVersion.mhasVersionPatch) {
        os << "." << pd.mVersion.mVersionPatch;
    }
    if (pd.mVersion.mhasVersionRepository) {
        os << "-" << pd.mVersion.mVersionRepository;
    }
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os,
        const std::vector<PackageDepend>& pds)
{
    std::vector<PackageDepend>::const_iterator itb = pds.begin();
    std::vector<PackageDepend>::const_iterator ite = pds.end();
    uint32_t stopcomma = pds.size()-1;

    for(unsigned int i=0; itb!=ite ; itb++, i++){
        os << (*itb);
        if(i<stopcomma){
            os << ", ";
        }
    }
    return os;
}

/**
 * \c RemotePackage stores a distant package data.
 *
 * \c RemotePackage have information about a distant package:
 * - description of the package's content.
 * - version of the package in a 3-uple, major, minor, patch.
 * - authors of the package.
 * - license of the package.
 * - an URL to the package source development.
 */
class RemotePackage
{
public:
    RemotePackage(std::istream& is, const std::string& url) :
        mSource(), mVersionMajor(0), mVersionMinor(0), mVersionPatch(0),
        mVersionRepository(0), mSection("no section"), mMaintainer(""),
        mHomepage(""), mUrl(url), mBuildDepends(), mDepends(), mConflicts(),
        mDescription("")
    {

        std::cout << " RemotePackage " << url << std::endl;

        try {
            std::string line;
            std::getline(is, line);

            if (isSource(line)) {
                setSource(std::string(line, 7));
                std::getline(is, line);
            }
            if (isVersion(line)) {
                setVersion(std::string(line, 9));
                std::getline(is, line);
            }
            if (isSection(line)) {
                setSection(std::string(line, 8));
                std::getline(is, line);
            }
            if (isMaintainer(line)) {
                setMaintainer(std::string(line, 11));
                std::getline(is, line);
            }
            if (isHomepage(line)) {
                setHomepage(std::string(line, 9));
                std::getline(is, line);
            }
            if (isUrl(line)) {
                setUrl(std::string(line, 5));
                std::getline(is, line);
            }
            if (isBuildDepends(line)) {
                setBuildDepends(std::string(line, 14));
                std::getline(is, line);
            }
            if (isDepends(line)) {
                setDepends(std::string(line, 8));
                std::getline(is, line);
            }
            if (isConflicts(line)) {
                setConflicts(std::string(line, 10));
                std::getline(is, line);
            }
            if (isDescription(line)) {
                setDescription(std::string(line, 12));
                std::getline(is, line);
                while (is and not isEndDescription(line)) {
                    appendDesciption(line);
                    std::getline(is, line);
                }
            }
        } catch (const std::ios_base::failure& /*e*/) {
        }
    }

    RemotePackage(const RemotePackage& other) :
        mSource(other.mSource), mVersionMajor(other.mVersionMajor),
        mVersionMinor(other.mVersionMinor), mVersionPatch(other.mVersionPatch),
        mVersionRepository(other.mVersionRepository), mSection(other.mSection),
        mMaintainer(other.mMaintainer), mHomepage(other.mHomepage),
        mUrl(other.mUrl), mBuildDepends(), mDepends(), mConflicts(),
        mDescription(other.mDescription)
    {
        {
            std::vector<PackageDepend>::const_iterator itb =
                    other.mBuildDepends.begin();
            std::vector<PackageDepend>::const_iterator ite =
                    other.mBuildDepends.end();
            for (;itb!=ite; itb ++){
                mBuildDepends.push_back((*itb));
            }
        }
        {
            std::vector<PackageDepend>::const_iterator itb =
                    other.mDepends.begin();
            std::vector<PackageDepend>::const_iterator ite =
                    other.mDepends.end();
            for (;itb!=ite; itb ++){
                mDepends.push_back((*itb));
            }
        }
        {
            std::vector<PackageDepend>::const_iterator itb =
                    other.mConflicts.begin();
            std::vector<PackageDepend>::const_iterator ite =
                    other.mConflicts.end();
            for (;itb!=ite; itb ++){
                mConflicts.push_back((*itb));
            }
        }
    }

    ~RemotePackage()
    {
    }

    RemotePackage& operator=(const RemotePackage& other)
    {
        RemotePackage tmp(other);
        tmp.swap(*this);
        return *this;
    }

    void swap(RemotePackage& other)
    {
        std::swap(mSource, other.mSource);
        std::swap(mVersionMajor, other.mVersionMajor);
        std::swap(mVersionMinor, other.mVersionMinor);
        std::swap(mVersionPatch, other.mVersionPatch);
        std::swap(mVersionRepository, other.mVersionRepository);
        std::swap(mSection, other.mSection);
        std::swap(mMaintainer, other.mMaintainer);
        std::swap(mHomepage, other.mHomepage);
        std::swap(mBuildDepends, other.mBuildDepends);
        std::swap(mDepends, other.mDepends);
        std::swap(mConflicts, other.mConflicts);
        std::swap(mDescription, other.mDescription);
        std::swap(mUrl, other.mUrl);
    }

    void setSource(const std::string& name)
    {
        std::string tmp = boost::algorithm::trim_copy(name);

        if (tmp.empty()) {
            throw utils::ArgError(fmt(_("RemotePackage: empty name")));
        } else {
            mSource = tmp;
        }
    }

    /**
     * Set version from a string
     * @param line, string representation of a version e.g "1.2.3"
     */
    void setVersion(const std::string& line)
    {
        PackageVersion pkgver;
        pkgver.fill(line);
        mVersionMajor = pkgver.mVersionMajor;
        mVersionMinor = pkgver.mVersionMinor;
        mVersionPatch = pkgver.mVersionPatch;
        mVersionRepository = pkgver.mVersionRepository;

    }
    void setVersion(uint32_t major, uint32_t minor, uint32_t patch)
    {
        mVersionMajor = major;
        mVersionMinor = minor;
        mVersionPatch = patch;
    }

    void setSection(const std::string& name)
    {
        std::string tmp = boost::algorithm::trim_copy(name);
        mSection = tmp;
    }

    void setMaintainer(const std::string& maintainer)
    {
        std::string tmp = boost::algorithm::trim_copy(maintainer);

        if (tmp.empty()) {
            throw utils::ArgError(fmt(_("RemotePackage: empty maintainer")));
        } else {
            mMaintainer = tmp;
        }
    }

    void setHomepage(const std::string& name)
    {
        std::string tmp = boost::algorithm::trim_copy(name);
        mHomepage = tmp;
    }

    void setUrl(const std::string& name)
    {
        std::string tmp = boost::algorithm::trim_copy(name);
        mUrl = tmp;
    }

    void setBuildDepends(const std::string& line)
    {
        std::vector < std::string > depends;

        boost::algorithm::split(depends, line,
                                boost::algorithm::is_any_of(","),
                                boost::algorithm::token_compress_on);

        setBuildDepends(depends);
    }

    void setBuildDepends(const std::vector < std::string >& depends)
    {
        typedef std::vector < std::string >::const_iterator iterator;

        mBuildDepends.clear();

        for (iterator it = depends.begin(); it != depends.end(); ++it) {
            std::string str = boost::algorithm::trim_copy(*it);
            if (not str.empty()) {
                PackageDepend pkgdep;
                pkgdep.fill(str);
                mBuildDepends.push_back(pkgdep);
            }
        }
    }

    void setDepends(const std::string& line)
    {
        std::vector < std::string > depends;

        boost::algorithm::split(depends, line,
                                boost::algorithm::is_any_of(","),
                                boost::algorithm::token_compress_on);

        setDepends(depends);
    }

    void setDepends(const std::vector < std::string >& depends)
    {
        typedef std::vector < std::string >::const_iterator iterator;
        mDepends.clear();
        for (iterator it = depends.begin(); it != depends.end(); ++it) {
            std::string str = boost::algorithm::trim_copy(*it);
            if (not str.empty()) {
                PackageDepend pkgdep;
                pkgdep.fill(str);
                mDepends.push_back(pkgdep);
            }
        }
    }

    void setConflicts(const std::string& line)
    {
        std::vector < std::string > depends;

        boost::algorithm::split(depends, line,
                                boost::algorithm::is_any_of(","),
                                boost::algorithm::token_compress_on);

        setConflicts(depends);
    }

    void setConflicts(const std::vector < std::string >& depends)
    {
        typedef std::vector < std::string >::const_iterator iterator;
        mConflicts.clear();
        for (iterator it = depends.begin(); it != depends.end(); ++it) {
            std::string str = boost::algorithm::trim_copy(*it);
            if (not str.empty()) {
                PackageDepend pkgdep;
                pkgdep.fill(str);

                mConflicts.push_back(pkgdep);
            }
        }
    }

    void setDescription(const std::string& description)
    {
        std::string tmp = boost::algorithm::trim_copy(description);

        if (tmp.empty()) {
            throw utils::ArgError(fmt(_("RemotePackage: empty description")));
        } else {
            mDescription = tmp;
        }
    }

    void appendDesciption(const std::string& description)
    {
        std::string tmp = boost::algorithm::trim_copy(description);

        mDescription.append(tmp);
    }

    std::string getSourcePackageUrl() const
    {
        return (fmt("%1%/%2%-%3%.%4%.zip") %
                mUrl % mSource % mVersionMajor % mVersionMinor).str();
    }

    std::string getBinaryPackageUrl() const
    {
        return (fmt("%1%/%2%-%3%.%4%.%5%-%6%-%7%.zip") %
                mUrl % mSource % mVersionMajor % mVersionMinor %
                mVersionPatch % VLE_SYSTEM_NAME % VLE_SYSTEM_PROCESSOR).str();
    }

    std::string mSource;
    uint32_t mVersionMajor;
    uint32_t mVersionMinor;
    uint32_t mVersionPatch;
    uint32_t mVersionRepository;
    std::string mSection;
    std::string mMaintainer;
    std::string mHomepage;
    std::string mUrl;
    std::vector<PackageDepend> mBuildDepends;
    std::vector<PackageDepend> mDepends;
    std::vector<PackageDepend> mConflicts;
    std::string mDescription;

};

bool operator==(const RemotePackage& a, const RemotePackage& b)
{
    return a.mSource == b.mSource
        and a.mVersionMajor == b.mVersionMajor
        and a.mVersionMinor == b.mVersionMinor
        and a.mVersionPatch == b.mVersionPatch;
}

bool operator<(const RemotePackage& a, const RemotePackage& b)
{
    if (a.mSource == b.mSource) {
        if (a.mVersionMajor == b.mVersionMajor) {
            if (a.mVersionMinor == b.mVersionMinor) {
                if (a.mVersionPatch == b.mVersionPatch) {
                    return false;
                } else {
                    return a.mVersionPatch < b.mVersionPatch;
                }
            } else {
                return a.mVersionMinor < b.mVersionMinor;
            }
        } else {
            return a.mVersionMajor < b.mVersionMajor;
        }
    } else {
        return a.mSource < b.mSource;
    }
}

std::ostream& operator<<(std::ostream& os, const RemotePackage& b)
{

    return os
        << "Source: " <<  b.mSource << "\n"
        << "Version: " << b.mVersionMajor
        << "." << b.mVersionMinor
        << "." << b.mVersionPatch
        << "-" << b.mVersionRepository << "\n"
        << "Section: " <<  b.mSection << "\n"
        << "Maintainer: " <<  b.mMaintainer << "\n"
        << "Homepage: " <<  b.mHomepage << "\n"
        << "Url: " <<  b.mUrl << "\n"
        << "Build-Depends: " << b.mBuildDepends << "\n"
        << "Depends: " << b.mDepends << "\n"
        << "Conflicts: " << b.mConflicts << "\n"
        << "Description: " << b.mDescription << "\n" << " ." << "\n";
}

class RemoteManager::Pimpl
{
public:
    typedef boost::unordered_map < std::string, RemotePackage > Pkgs;
    typedef Pkgs::const_iterator const_iterator;
    typedef Pkgs::iterator iterator;

    Pimpl()
        : mStream(0), mIsStarted(false), mIsFinish(false), mStop(false),
        mHasError(false)
    {
        read(buildPackageFilename(),"");
    }

    ~Pimpl()
    {
        join();
    }

    void start(RemoteManagerActions action, const std::string& arg,
               std::ostream* out)
    {
        if (not mIsStarted) {
            mIsStarted = true;
            mIsFinish = false;
            mStop = false;
            mHasError = false;
            mArgs = arg;
            mStream = out;

            switch (action) {
            case REMOTE_MANAGER_UPDATE:
                mThread = boost::thread(
                    &RemoteManager::Pimpl::actionUpdate, this);
                break;
            case REMOTE_MANAGER_SOURCE:
                mThread = boost::thread(
                    &RemoteManager::Pimpl::actionSource, this);
                break;
            case REMOTE_MANAGER_INSTALL:
                mThread = boost::thread(
                    &RemoteManager::Pimpl::actionInstall, this);
                break;
            case REMOTE_MANAGER_SEARCH:
                mThread = boost::thread(
                    &RemoteManager::Pimpl::actionSearch, this);
                break;
            case REMOTE_MANAGER_SHOW:
                mThread = boost::thread(
                    &RemoteManager::Pimpl::actionShow, this);
                break;
            default:
                break;
            }
        }
    }

    void join()
    {
        if (mIsStarted) {
            if (not mIsFinish) {
                mThread.join();
            }
        }
    }

    void stop()
    {
        if (mIsStarted and not mIsFinish) {
            mStop = true;
        }
    }

private:
    /**
     * Build the filename of the user's packages in \c VLE_HOME/packages.
     *
     * @return A filename.
     */
    static std::string buildPackageFilename()
    {
        return utils::Path::path().getHomeFile("packages");
    }

    /**
     * Send the parameter of the template function \c t to
     *
     * @param t
     */
    template < typename T > void out(const T& t)
    {
        if (mStream) {
            *mStream << t;
        }
    }

    /**
     * A functor to check if a @c Packages::value_type corresponds to the
     * regular expression provided in constructor.
     */
    struct HaveExpression
    {
        HaveExpression(const boost::regex& expression,
                       std::ostream& stream)
            : expression(expression), stream(stream)
        {
        }

        void operator()(const Pkgs::value_type& value) const
        {
            boost::sregex_iterator it(value.first.begin(),
                                      value.first.end(),
                                      expression);
            boost::sregex_iterator end;

            if (it != end) {
                stream << value.first << "\n";
            } else {
                boost::sregex_iterator jt(value.second.mDescription.begin(),
                                          value.second.mDescription.end(),
                                          expression);

                if (jt != end) {
                    stream << value.first << "\n";
                }
            }
        }

        const boost::regex& expression;
        std::ostream& stream;
    };

    /**
     * Read the package file \c filename.
     *
     * @param[in] The filename to read.
     * @param[in] The url the package file comes from.
     */
    void read(const std::string& filename,
            const std::string& url)
    {
        std::ifstream file(filename.c_str());
        if (file) {
            file.exceptions(std::ios_base::eofbit | std::ios_base::failbit |
                            std::ios_base::badbit);

            while (not file.eof()) {
                RemotePackage pkg(file, url);
                if (not file.eof()) {
                    std::cout << " DBG " << pkg.mSource << std::endl;
                    mPackages.insert(
                            std::make_pair < std::string, RemotePackage >(
                                    pkg.mSource, pkg));
                }

            }
        } else {
            TraceAlways(fmt(_("Failed to open package file `%1%'")) % filename);
        }
    }

    /**
     * Write the default package \c VLE_HOME/packages.
     */
    void save() const throw()
    {
        std::ofstream file(buildPackageFilename().c_str());

        if (file) {
            file.exceptions(std::ios_base::eofbit | std::ios_base::failbit |
                            std::ios_base::badbit);

            try {
                std::transform(
                    mPackages.begin(), mPackages.end(),
                    std::ostream_iterator < RemotePackage >( file, ""),
                    select2nd < Pkgs::value_type >());
            } catch (const std::exception& /*e*/) {
                TraceAlways(fmt(_("Failed to write package file `%1%'")) %
                            buildPackageFilename());
            }
        } else {
            TraceAlways(fmt(_("Failed to open package file `%1%'")) %
                        buildPackageFilename());
        }
    }

    //
    // threaded slot
    //

    void actionUpdate() throw()
    {
        std::vector < std::string > urls;

        try {
            utils::Preferences prefs;
            std::string tmp;
            prefs.get("vle.remote.url", &tmp);

            boost::algorithm::split(urls, tmp,
                                    boost::algorithm::is_any_of(","),
                                    boost::algorithm::token_compress_on);
        } catch(const std::exception& /*e*/) {
            TraceAlways(_("Failed to read preferences file"));
        }

        out(_("Update database\n"));

        std::vector < std::string >::const_iterator it, end = urls.end();
        for (it = urls.begin(); it != end; ++it) {
            try {
                DownloadManager dl;

                std::string url = *it;
                std::string urlfile(url);
                urlfile += '/';
                urlfile += "packages";

                out(fmt(_("Download %1% ...")) % urlfile);

                dl.start(urlfile);
                dl.join();

                if (not dl.hasError()) {
                    out(_("ok"));
                    std::string filename(dl.filename());

                    try {
                        out(_("(merged: "));
                        read(filename, url);
                        out(_("ok)"));
                    } catch (...) {
                        out(_("failed)"));
                    }
                } else {
                    out(_("failed"));
                }
                out("\n");
            } catch (const std::exception& e) {
                out(fmt(_("failed `%1%'")) % e.what());
            }
        }

        out(_("Database updated\n"));

        save();

        mStream = 0;
        mIsFinish = true;
        mIsStarted = false;
        mStop = false;
        mHasError = false;
    }

    void actionInstall() throw()
    {
        const_iterator it = mPackages.find(mArgs);

        if (it != mPackages.end()) {
            std::string url = it->second.getBinaryPackageUrl();

            DownloadManager dl;

            out(fmt(_("Download binary package `%1%' at %2%")) % mArgs % url);
            dl.start(url);
            dl.join();

            if (not dl.hasError()) {
                out(_("install"));
                std::string filename = dl.filename();
                std::string zipfilename = dl.filename();
                zipfilename += ".zip";

                boost::filesystem::rename(filename, zipfilename);

                utils::Package::package().unzip(mArgs, zipfilename);
                utils::Package::package().wait(*mStream, *mStream);
                out(_(": ok\n"));
            } else {
                out(_(": failed\n"));
            }
        } else {
            out(fmt(_("Unknown package `%1%'")) % mArgs);
        }

        mStream = 0;
        mIsFinish = true;
        mIsStarted = false;
        mStop = false;
        mHasError = false;
    }

    void actionSource() throw()
    {



        const_iterator it = mPackages.find(mArgs);

        if (it != mPackages.end()) {
            std::string url = it->second.getSourcePackageUrl();

            DownloadManager dl;

            out(fmt(_("Download source package `%1%' at %2%")) % mArgs % url);
            dl.start(url);
            dl.join();

            if (not dl.hasError()) {
                out(_("install"));
                std::string filename = dl.filename();
                std::string zipfilename = dl.filename();
                zipfilename += ".zip";

                boost::filesystem::rename(filename, zipfilename);

                utils::Package::package().unzip(mArgs, zipfilename);
                utils::Package::package().wait(*mStream, *mStream);
                out(_(": ok\n"));
            } else {
                out(_(": failed\n"));
            }
        } else {
            out(fmt(_("Unknown package `%1%'")) % mArgs);
        }

        mStream = 0;
        mIsFinish = true;
        mIsStarted = false;
        mStop = false;
        mHasError = false;
    }

    void actionSearch() throw()
    {
        if (mStream) {
            boost::regex expression(mArgs, boost::regex::grep);

            std::for_each(mPackages.begin(),
                          mPackages.end(),
                          HaveExpression(expression, *mStream));
        }

        mStream = 0;
        mIsFinish = true;
        mIsStarted = false;
        mStop = false;
        mHasError = false;
    }

    void actionShow() throw()
    {
        const_iterator it = mPackages.find(mArgs);

        if (it != mPackages.end()) {
            out(it->second);
        } else {
            out(fmt(_("Unknown package `%1%'")) % mArgs);
        }

        mStream = 0;
        mIsFinish = true;
        mIsStarted = false;
        mStop = false;
        mHasError = false;
    }

    Pkgs mPackages;
    boost::mutex mMutex;
    boost::thread mThread;
    std::string mArgs;
    std::ostream* mStream;
    bool mIsStarted;
    bool mIsFinish;
    bool mStop;
    bool mHasError;
};

RemoteManager::RemoteManager()
    : mPimpl(new RemoteManager::Pimpl())
{
}

RemoteManager::~RemoteManager()
{
    delete mPimpl;
}

void RemoteManager::start(RemoteManagerActions action,
                          const std::string& arg,
                          std::ostream* os)
{
    mPimpl->start(action, arg, os);
}

void RemoteManager::join()
{
    mPimpl->join();
}

void RemoteManager::stop()
{
    mPimpl->stop();
}

}} // namespace vle utils
