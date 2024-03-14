include(FetchContent)

fetchcontent_declare(
  pt
  GIT_REPOSITORY https://github.com/ZhuLingQing/protothreads.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(pt)
