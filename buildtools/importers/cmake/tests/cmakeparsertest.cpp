/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakeparsertest.h"

#include <QtCore/QTemporaryFile>
#include "cmListFileLexer.h"

static bool parseCMakeFile( const char* fileName );

static bool parseCMakeFunction( cmListFileLexer* lexer,
                                const char* filename );

QTEST_MAIN( CMakeParserTest )

CMakeParserTest::CMakeParserTest()
{
}


CMakeParserTest::~CMakeParserTest()
{
}

void CMakeParserTest::testLexerCreation()
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    QVERIFY( lexer != 0 );
    cmListFileLexer_Delete( lexer );
}

void CMakeParserTest::testLexerWithFile()
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
         QFAIL( "Unable to open temporary file" );

    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file

    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        QFAIL( "unable to create lexer" );
    QVERIFY( cmListFileLexer_SetFileName( lexer, qPrintable( tempName ) ) );
    cmListFileLexer_Delete( lexer );
    tempFile.remove();
}

void CMakeParserTest::testParserWithGoodData()
{
//    QFAIL( "the magic is missing" );
    QFETCH( QString, text );
    QTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
        QFAIL( "Unable to open temporary file" );

    tempFile.write( text.toUtf8() );
    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file
    bool parseError = parseCMakeFile( qPrintable( tempName ) );
    QVERIFY( parseError == false );

}

void CMakeParserTest::testParserWithGoodData_data()
{
    QTest::addColumn<QString>( "text" );
    QTest::newRow( "good data1" ) << "project(foo)\nset(foobar_SRCS foo.h foo.c)";
    QTest::newRow( "good data2" ) << "set(foobar_SRCS foo.h foo.c)\n"
                                     "add_executable( foo ${foobar_SRCS})";
}

void CMakeParserTest::testParserWithBadData()
{
    QFETCH( QString, text );
    QTemporaryFile tempFile;
    tempFile.setAutoRemove( false );
    tempFile.open();
    if ( !QFile::exists( tempFile.fileName() ) )
        QFAIL( "Unable to open temporary file" );

    tempFile.write( text.toUtf8() );
    QString tempName = tempFile.fileName();
    tempFile.close(); //hacks to the get name of the file
    bool parseError = parseCMakeFile( qPrintable( tempName ) );
    QVERIFY( parseError == true );
}

void CMakeParserTest::testParserWithBadData_data()
{
    QTest::addColumn<QString>( "text" );

    //just plain wrong. :)
    QTest::newRow( "bad data 1" ) << "foo bar baz zippedy do dah";

    //missing right parenthesis
    QTest::newRow( "bad data 2" ) << "set(mysrcs_SRCS foo.c\n\n\n";

    //extra identifiers between functions. cmake doesn't allow plain identifiers
    QTest::newRow( "bad data 3" ) << "the(quick) brown fox jumped(over) the lazy dog";

    //invalid due to no newline before next command
    QTest::newRow( "bad data 4" ) << "project(foo) set(mysrcs_SRCS foo.c)";
}


bool parseCMakeFile( const char* fileName )
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        return false;
    if ( !cmListFileLexer_SetFileName( lexer, fileName ) )
        return false;

    bool parseError = false;
    bool haveNewline = true;
    cmListFileLexer_Token* token;
    while(!parseError && (token = cmListFileLexer_Scan(lexer)))
    {
        if(token->type == cmListFileLexer_Token_Newline)
        {
            haveNewline = true;
        }
        else if(token->type == cmListFileLexer_Token_Identifier)
        {
            if(haveNewline)
            {
                haveNewline = false;
                if ( parseCMakeFunction( lexer, fileName ) )
                    parseError = false;
                else
                    parseError = true;
            }
            else
                parseError = true;
        }
        else
            parseError = true;
    }

    return parseError;

}


bool parseCMakeFunction( cmListFileLexer* lexer,
                         const char* fileName )
{
    // Command name has already been parsed.  Read the left paren.
    cmListFileLexer_Token* token;
    if(!(token = cmListFileLexer_Scan(lexer)))
    {
        return false;
    }
    if(token->type != cmListFileLexer_Token_ParenLeft)
    {
        return false;
    }

    // Arguments.
    unsigned long lastLine = cmListFileLexer_GetCurrentLine(lexer);
    while((token = cmListFileLexer_Scan(lexer)))
    {
        if(token->type == cmListFileLexer_Token_ParenRight)
        {
            return true;
        }
        else if(token->type == cmListFileLexer_Token_Identifier ||
                token->type == cmListFileLexer_Token_ArgumentUnquoted)
        {
        }
        else if(token->type == cmListFileLexer_Token_ArgumentQuoted)
        {
        }
        else if(token->type != cmListFileLexer_Token_Newline)
        {
            return false;
        }
        lastLine = cmListFileLexer_GetCurrentLine(lexer);
    }

    return false;
}


#include "cmakeparsertest.moc"

