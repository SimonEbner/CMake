/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator3
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"

cmGlobalUnixMakefileGenerator3::cmGlobalUnixMakefileGenerator3()
{
  // This type of makefile always requires unix style paths
  m_ForceUnixPaths = true;
  m_FindMakeProgramFile = "CMakeUnixFindMake.cmake";
}

void cmGlobalUnixMakefileGenerator3
::EnableLanguage(std::vector<std::string>const& languages, cmMakefile *mf)
{
  mf->AddDefinition("CMAKE_CFG_INTDIR",".");
  this->cmGlobalGenerator::EnableLanguage(languages, mf);
  std::string path;
  for(std::vector<std::string>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    const char* lang = l->c_str();
    std::string langComp = "CMAKE_";
    langComp += lang;
    langComp += "_COMPILER";
    
    if(!mf->GetDefinition(langComp.c_str()))
      {
      cmSystemTools::Error(langComp.c_str(), " not set, after EnableLanguage");
      continue;
      }
    const char* cc = mf->GetRequiredDefinition(langComp.c_str());
    path = cmSystemTools::FindProgram(cc);
    if(path.size() == 0)
      {
      std::string message = "your ";
      message += lang;
      message += " compiler: ";
      if(cc)
        {
        message +=  cc;
        }
      else
        {
        message += "(NULL)";
        }
      message += " was not found in your path.   "
        "For CMake to correctly use try compile commands, the compiler must "
        "be in your path.   Please add the compiler to your PATH environment,"
        " and re-run CMake.";
        cmSystemTools::Error(message.c_str());
      }
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalUnixMakefileGenerator3::CreateLocalGenerator()
{
  cmLocalGenerator* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalUnixMakefileGenerator3::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates standard UNIX makefiles.";
  entry.full =
    "A hierarchy of UNIX makefiles is generated into the build tree.  Any "
    "standard UNIX-style make program can build the project through the "
    "default make target.  A \"make install\" target is also provided.";
}

//----------------------------------------------------------------------------
void cmGlobalUnixMakefileGenerator3::WriteMainCMakefile()
{
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string cmakefileName = this->GetCMakeInstance()->GetHomeOutputDirectory();
  cmakefileName += "/Makefile.cmake";
  std::string makefileName = this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/Makefile";
  cmGeneratedFileStream cmakefileStream(cmakefileName.c_str());
  if(!cmakefileStream)
    {
    return;
    }
 
  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3 *lg = 
    static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[0]);
    
  // Write the do not edit header.
  lg->WriteDisclaimer(cmakefileStream);

  // for each cmMakefile get its list of dependencies
  unsigned int i;
  std::vector<std::string> lfiles;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    lg = static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
  
    // Get the list of files contributing to this generation step.
    lfiles.insert(lfiles.end(),lg->GetMakefile()->GetListFiles().begin(),
                  lg->GetMakefile()->GetListFiles().end());
    }
  // Sort the list and remove duplicates.
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>());
  std::vector<std::string>::iterator new_end = 
    std::unique(lfiles.begin(),lfiles.end());
  lfiles.erase(new_end, lfiles.end());

  // reset lg to the first makefile
  lg = static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[0]);

  // Build the path to the cache file.
  std::string cache = this->GetCMakeInstance()->GetHomeOutputDirectory();
  cache += "/CMakeCache.txt";

  // Save the list to the cmake file.
  cmakefileStream
    << "# The top level Makefile was generated from the following files:\n"
    << "SET(CMAKE_MAKEFILE_DEPENDS\n"
    << "  \"" << lg->ConvertToRelativePath(cache.c_str()).c_str() << "\"\n";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    cmakefileStream
      << "  \"" << lg->ConvertToRelativePath(i->c_str()).c_str()
      << "\"\n";
    }
  cmakefileStream
    << "  )\n\n";

  // Build the path to the cache check file.
  std::string check = this->GetCMakeInstance()->GetHomeOutputDirectory();
  check += "/cmake.check_cache";

  // Set the corresponding makefile in the cmake file.
  cmakefileStream
    << "# The corresponding makefile is:\n"
    << "SET(CMAKE_MAKEFILE_OUTPUTS\n"
    << "  \"" << lg->ConvertToRelativePath(makefileName.c_str()).c_str() << "\"\n"
    << "  \"" << lg->ConvertToRelativePath(check.c_str()).c_str() << "\"\n";

  // add in all the directory information files
  std::string tmpStr;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    lg = static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
    tmpStr = lg->GetMakefile()->GetStartOutputDirectory();
    tmpStr += "/CMakeDirectoryInformation.cmake";
    cmakefileStream 
      << "  \"" << this->ConvertToHomeRelativePath(tmpStr.c_str()).c_str() << "\"\n";
    }
  cmakefileStream << "  )\n\n";
  
  // now write all the language stuff
  // Set the set of files to check for dependency integrity.
  // loop over all of the local generators to collect this
  std::set<cmStdString> checkSetLangs;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    lg = static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
    std::map<cmStdString,cmLocalUnixMakefileGenerator3::IntegrityCheckSet>& checkSet = 
      lg->GetIntegrityCheckSet();
    for(std::map<cmStdString, 
          cmLocalUnixMakefileGenerator3::IntegrityCheckSet>::const_iterator
          l = checkSet.begin(); l != checkSet.end(); ++l)
      {
      checkSetLangs.insert(l->first);
      }
    }
  
  // list the languages
  cmakefileStream
    << "# The set of files whose dependency integrity should be checked:\n";
  cmakefileStream
    << "SET(CMAKE_DEPENDS_LANGUAGES\n";
  for(std::set<cmStdString>::iterator
        l = checkSetLangs.begin(); l != checkSetLangs.end(); ++l)
    {
    cmakefileStream << "  \"" << l->c_str() << "\"\n";
    }
  cmakefileStream << "  )\n";
  
  // now list the files for each language
  for(std::set<cmStdString>::iterator
        l = checkSetLangs.begin(); l != checkSetLangs.end(); ++l)
    {
    cmakefileStream
      << "SET(CMAKE_DEPENDS_CHECK_" << l->c_str() << "\n";
    // now for each local gen get the checkset
    for (i = 0; i < m_LocalGenerators.size(); ++i)
      {
      lg = static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
      // get the check set for this local gen and language
      cmLocalUnixMakefileGenerator3::IntegrityCheckSet iCheckSet = 
        lg->GetIntegrityCheckSet()[*l];
      // for each file
      for(cmLocalUnixMakefileGenerator3::IntegrityCheckSet::const_iterator csIter = 
            iCheckSet.begin();
          csIter != iCheckSet.end(); ++csIter)
        {
        cmakefileStream
          << "  \"" << this->ConvertToHomeRelativePath(csIter->c_str()).c_str() << "\"\n";
        }
      }
    cmakefileStream << "  )\n";
    }
  
}

void cmGlobalUnixMakefileGenerator3::WriteMainMakefile()
{
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string makefileName = this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/Makefile";
  cmGeneratedFileStream makefileStream(makefileName.c_str());
  if(!makefileStream)
    {
    return;
    }
 
  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3 *lg = 
    static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[0]);
    
  // Write the do not edit header.
  lg->WriteDisclaimer(makefileStream);
  
  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.push_back("all");

  // Write the rule.
  lg->WriteMakeRule(makefileStream,
                    "Default target executed when no arguments are "
                    "given to make.",
                    "default_target",
                    depends,
                    no_commands);

  lg->WriteMakeVariables(makefileStream);
  
  lg->WriteSpecialTargetsTop(makefileStream);

  lg->WriteAllRules(makefileStream);
  
  // Write special "cmake_check_build_system" target to run cmake with
  // the --check-build-system flag.
  // Build command to run CMake to check if anything needs regenerating.
  std::string cmakefileName = makefileName;
  cmakefileName += ".cmake";
  std::string runRule = this->GetCMakeInstance()->GetCacheDefinition("CMAKE_COMMAND");
  runRule += " -H";
  runRule += this->GetCMakeInstance()->GetHomeDirectory();
  runRule += " -B";
  runRule += this->GetCMakeInstance()->GetHomeOutputDirectory();
  runRule += " --check-build-system ";
  runRule += lg->ConvertToRelativeOutputPath(cmakefileName.c_str());

  std::vector<std::string> no_depends;
  std::vector<std::string> commands;
  commands.push_back(runRule);
  lg->WriteMakeRule(makefileStream,
                    "Special rule to run CMake to check the build system "
                    "integrity.\n"
                    "No rule that depends on this can have "
                    "commands that come from listfiles\n"
                    "because they might be regenerated.",
                    "cmake_check_build_system",
                    no_depends,
                    commands);

  // write the target convenience rules
  unsigned int i;
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    lg = static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
    lg->WriteConvenienceRules(makefileStream);
    }
}

void cmGlobalUnixMakefileGenerator3::WriteSupportMakefiles()
{
  this->WriteDependMakefile();
  this->WriteBuildMakefile();
  this->WriteCleanMakefile();
}

void cmGlobalUnixMakefileGenerator3::WriteDependMakefile()
{
  unsigned int i;
  
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string makefileName = this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/depend.make";
  cmGeneratedFileStream makefileStream(makefileName.c_str());
  if(!makefileStream)
    {
    return;
    }
  
  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3 *lg = 
    static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[0]);
  
  // Write the do not edit header.
  lg->WriteDisclaimer(makefileStream);
  lg->WriteMakeVariables(makefileStream);

  // add the generic dependency
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  lg->WriteMakeRule(makefileStream, 0, "depend", depends, no_commands);

  // include the build rules
  makefileStream
    << "# Include make rules for build targets\n";
  makefileStream
    << lg->GetIncludeDirective() << " "
    << lg->ConvertToOutputForExisting("build.make").c_str()
    << "\n\n";

  // include all the target depends
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmLocalUnixMakefileGenerator3 *lg2 = 
      static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
    if (!lg2->GetExcludeAll())
      {
      lg2->WriteTargetIncludes(makefileStream,"depend.make","depend");
      }
    }
}

void cmGlobalUnixMakefileGenerator3::WriteBuildMakefile()
{
  unsigned int i;
  
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string makefileName = this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/build.make";
  cmGeneratedFileStream makefileStream(makefileName.c_str());
  if(!makefileStream)
    {
    return;
    }
  
  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3 *lg = 
    static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[0]);
  
  // Write the do not edit header.
  lg->WriteDisclaimer(makefileStream);
  lg->WriteMakeVariables(makefileStream);

  // add the generic dependency
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  lg->WriteMakeRule(makefileStream, 0, "build", depends, no_commands);

  // include all the target depends
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmLocalUnixMakefileGenerator3 *lg2 = 
      static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
    // are any parents excluded
    bool exclude = false;
    cmLocalGenerator *lg3 = lg2;
    while (lg3)
      {
      if (lg3->GetExcludeAll())
        {
        exclude = true;
        break;
        }
      lg3 = lg3->GetParent();
      }
    if (!exclude)
      {
      lg2->WriteTargetIncludes(makefileStream,"build.make","build");
      }
    }
}

void cmGlobalUnixMakefileGenerator3::WriteCleanMakefile()
{
  unsigned int i;
  
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string makefileName = this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/clean.make";
  cmGeneratedFileStream makefileStream(makefileName.c_str());
  if(!makefileStream)
    {
    return;
    }
  
  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3 *lg = 
    static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[0]);
  
  // Write the do not edit header.
  lg->WriteDisclaimer(makefileStream);
  lg->WriteMakeVariables(makefileStream);

  // add the generic dependency
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  lg->WriteMakeRule(makefileStream, 0, "clean", depends, no_commands);

  // include all the target depends
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmLocalUnixMakefileGenerator3 *lg2 = 
      static_cast<cmLocalUnixMakefileGenerator3 *>(m_LocalGenerators[i]);
    lg2->WriteTargetIncludes(makefileStream,"clean.make","clean");
    // add the directory based rules
    lg2->WriteLocalCleanRule(makefileStream);
    }
  
}

//----------------------------------------------------------------------------
void cmGlobalUnixMakefileGenerator3::Generate() 
{
  // first do superclass method
  this->cmGlobalGenerator::Generate();

  // write the main makefile
  this->WriteMainMakefile();
  this->WriteMainCMakefile();

  // now write the support Makefiles
  this->WriteSupportMakefiles();
}
