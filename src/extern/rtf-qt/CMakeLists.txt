include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
set(CMAKE_AUTOMOC ON)

set(RtfReader_LIB_SOURCES
	rtfreader.cpp
	controlword.cpp
	AbstractRtfOutput.cpp
	AuthorPcdataDestination.cpp
	CategoryPcdataDestination.cpp
	ColorTableDestination.cpp
	CommentPcdataDestination.cpp
	CompanyPcdataDestination.cpp
	Destination.cpp
	DocumentCommentPcdataDestination.cpp
	DocumentDestination.cpp
	FontTableDestination.cpp
	GeneratorPcdataDestination.cpp
	HLinkBasePcdataDestination.cpp
	IgnoredDestination.cpp
	InfoDestination.cpp
	InfoTimeDestination.cpp
	InfoCreatedTimeDestination.cpp
	InfoPrintedTimeDestination.cpp
	InfoRevisedTimeDestination.cpp
	KeywordsPcdataDestination.cpp
	ManagerPcdataDestination.cpp
	ODFOutput.cpp
	OperatorPcdataDestination.cpp
	PcdataDestination.cpp
	PictDestination.cpp
	SubjectPcdataDestination.cpp
	StyleSheetDestination.cpp
	TextDocumentRtfOutput.cpp
	TitlePcdataDestination.cpp
	Token.cpp
	Tokenizer.cpp
	UserPropsDestination.cpp
    FieldDestination.cpp
    HtmlOutput.cpp
    TheWordRtfOutput.cpp
    ESwordRtfOutput.cpp
)


add_library(RtfReader STATIC ${RtfReader_LIB_SOURCES})
target_link_libraries(RtfReader Qt6::Core Qt6::Gui)
set_target_properties(RtfReader PROPERTIES VERSION 0.0.0 SOVERSION 0)
