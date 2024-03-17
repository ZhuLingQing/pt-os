include(FetchContent)

fetchcontent_declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl.git
  GIT_TAG        20.38.10
)

FetchContent_MakeAvailable(etl)
