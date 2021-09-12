
find_package(Git)

if( Git_FOUND )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${AUDIOAPPSRC}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${AUDIOAPPSRC}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
  message( WARNING "Git isn't present in your path. Setting hashes to defaults" )
  set( GIT_BRANCH "git-not-present" )
  set( GIT_COMMIT_HASH "git-not-present" )
endif()

if( WIN32 )
  set( AUDIOAPP_BUILD_ARCH "Intel" )
else()
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE AUDIOAPP_BUILD_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

cmake_host_system_information(RESULT AUDIOAPP_BUILD_FQDN QUERY FQDN )

message( STATUS "Setting up audioapp version" )
message( STATUS "  git hash is ${GIT_COMMIT_HASH} and branch is ${GIT_BRANCH}" )
message( STATUS "  buildhost is ${AUDIOAPP_BUILD_FQDN}" )
message( STATUS "  buildarch is ${AUDIOAPP_BUILD_ARCH}" )

if( ${AZURE_PIPELINE} )
  message( STATUS "Azure Pipeline Build" )
  set( lpipeline "pipeline" )
else()
  message( STATUS "Developer Local Build" )
  set( lpipeline "local" )
endif()

if(${GIT_BRANCH} STREQUAL "main" )
  if( ${AZURE_PIPELINE} )
    set( lverpatch "nightly" )
  else()
    set( lverpatch "main" )
  endif()
  set( lverrel "999" )
  set( fverpatch ${lverpatch} )
else()
  string( FIND ${GIT_BRANCH} "release/" RLOC )
  if( ${RLOC} EQUAL 0 )
    message( STATUS "Configuring a Release Build from '${GIT_BRANCH}'" )
    string( SUBSTRING ${GIT_BRANCH} 11 100 RV ) # that's release slash 1.7.
    string( FIND ${RV} "." DLOC )
    if( NOT ( DLOC EQUAL -1 ) )
      math( EXPR DLP1 "${DLOC} + 1" )
      string( SUBSTRING ${RV} ${DLP1} 100 LRV ) # skip that first dots
      set( lverrel ${LRV} )
    else()
      set( lverrel "99" )
    endif()
    set( lverpatch "stable-${lverrel}" )
    set( fverpatch "${lverrel}" )
  else()
    set( lverpatch ${GIT_BRANCH} )
    set( fverpatch ${lverpatch} )
    set( lverrel "1000" )
  endif()
endif()

set( AUDIOAPP_FULL_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${fverpatch}.${GIT_COMMIT_HASH}" )
set( AUDIOAPP_MAJOR_VERSION "${PROJECT_VERSION_MAJOR}" )
set( AUDIOAPP_SUB_VERSION "${PROJECT_VERSION_MINOR}" )
set( AUDIOAPP_RELEASE_VERSION "${lverpatch}" )
set( AUDIOAPP_RELEASE_NUMBER "${lverrel}" )
set( AUDIOAPP_BUILD_HASH "${GIT_COMMIT_HASH}" )
set( AUDIOAPP_BUILD_LOCATION "${lpipeline}" )

string( TIMESTAMP AUDIOAPP_BUILD_DATE "%Y-%m-%d" )
string( TIMESTAMP AUDIOAPP_BUILD_TIME "%H:%M:%S" )

message( STATUS "Using AUDIOAPP_VERSION=${AUDIOAPP_FULL_VERSION}" )

message( STATUS "Configuring ${AUDIOAPPBLD}/geninclude/version.cpp" )
configure_file( ${AUDIOAPPSRC}/src/version.cpp.in
  ${AUDIOAPPBLD}/geninclude/version.cpp )

