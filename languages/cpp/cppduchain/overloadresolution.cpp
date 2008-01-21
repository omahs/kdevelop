/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "overloadresolution.h"
#include "cppduchain/typeutils.h"
#include "duchain/ducontext.h"
#include "duchain/declaration.h"
#include "duchain/classfunctiondeclaration.h"
#include "cppduchain/cpptypes.h"
#include "viablefunctions.h"
#include <QtAlgorithms>

using namespace Cpp;
using namespace KDevelop;

OverloadResolver::OverloadResolver( DUContextPointer context, TopDUContextPointer topContext  ) : m_context(context), m_topContext(topContext), m_worstConversionRank(NoMatch) {
  
}

Declaration* OverloadResolver::resolveConstructor( const ParameterList& params, bool implicit, bool noUserDefinedConversion ) {
    if( !m_context || !m_topContext )
      return 0;
    
    QList<Declaration*> goodDeclarations;
    QList<Declaration*> declarations = m_context->findLocalDeclarations(m_context->localScopeIdentifier(), KDevelop::SimpleCursor(), m_topContext.data());
    
    for( QList<Declaration*>::iterator it = declarations.begin(); it != declarations.end(); ++it ) {
      if( (*it)->abstractType() )
      {
        CppFunctionType* function = dynamic_cast<CppFunctionType*>( (*it)->abstractType().data() );
        ClassFunctionDeclaration* functionDeclaration = dynamic_cast<ClassFunctionDeclaration*>(*it);
        //Q_ASSERT();
        
        if( functionDeclaration && functionDeclaration->isConstructor() )
        {
          if( function->arguments().size() >= params.parameters.size() )
          {
            if( !implicit || !functionDeclaration->isExplicit() )
              goodDeclarations << *it;
          }
        }
      }
    }

    return resolveList( params, goodDeclarations, noUserDefinedConversion );
}

Declaration* OverloadResolver::resolve( const ParameterList& params, const QualifiedIdentifier& functionName, bool noUserDefinedConversion )
{
  if( !m_context || !m_topContext )
    return 0;
  
  QList<Declaration*> declarations = m_context->findDeclarations(functionName, KDevelop::SimpleCursor(), AbstractType::Ptr(), m_topContext.data());
  
  return resolveList(params, declarations, noUserDefinedConversion );
}

uint OverloadResolver::worstConversionRank() {
  return m_worstConversionRank;
}

void OverloadResolver::expandDeclarations( const QList<Declaration*>& declarations, QList<Declaration*>& newDeclarations ) {
  for( QList<Declaration*>::const_iterator it = declarations.begin(); it != declarations.end(); ++it ) {
    Declaration* decl = *it;
    bool isConstant = false;

    if( CppClassType* klass = dynamic_cast<CppClassType*>( TypeUtils::realType(decl->abstractType(), m_topContext.data(), &isConstant) ) )
    {
      if( decl->kind() == Declaration::Instance ) {
        //Instances of classes should be substituted with their operator() members
        TypeUtils::getMemberFunctions( klass, m_topContext.data(), newDeclarations, "operator()", isConstant );
      } else {
        //Classes should be substituted with their constructors
        TypeUtils::getConstructors( klass, m_topContext.data(), newDeclarations );
      }
    }
  }
}

void OverloadResolver::expandDeclarations( const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, QList<QPair<OverloadResolver::ParameterList, Declaration*> >& newDeclarations ) {
  for( QList<QPair<OverloadResolver::ParameterList, Declaration*> >::const_iterator it = declarations.begin(); it != declarations.end(); ++it ) {
    QPair<OverloadResolver::ParameterList, Declaration*> decl = *it;
    bool isConstant = false;

    if( CppClassType* klass = dynamic_cast<CppClassType*>( TypeUtils::realType(decl.second->abstractType(), m_topContext.data(), &isConstant) ) )
    {
      if( decl.second->kind() == Declaration::Instance ) {
        //Instances of classes should be substituted with their operator() members
        QList<Declaration*> functions;
        TypeUtils::getMemberFunctions( klass, m_topContext.data(), functions, "operator()", isConstant );
        foreach(Declaration* f, functions)
          newDeclarations << QPair<OverloadResolver::ParameterList, Declaration*>(decl.first, f );
      } else {
        //Classes should be substituted with their constructors
        QList<Declaration*> functions;
        TypeUtils::getConstructors( klass, m_topContext.data(), functions );
        foreach(Declaration* f, functions)
          newDeclarations << QPair<OverloadResolver::ParameterList, Declaration*>( decl.first, f );
      }
    }
  }
}

Declaration* OverloadResolver::resolveList( const ParameterList& params, const QList<Declaration*>& declarations, bool noUserDefinedConversion )
{
  if( !m_context || !m_topContext )
    return 0;
  
  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;


  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QList<Declaration*> newDeclarations = declarations;
  expandDeclarations( declarations, newDeclarations );
  
  ///Second step: Find best viable function
  ViableFunction bestViableFunction( m_topContext.data() );
  
  for( QList<Declaration*>::const_iterator it = newDeclarations.begin(); it != newDeclarations.end(); ++it )
  {
    ViableFunction viable( m_topContext.data(), *it , noUserDefinedConversion );
    viable.matchParameters( params );
    
    if( viable.isBetter(bestViableFunction) ) {
      bestViableFunction = viable;
      m_worstConversionRank = bestViableFunction.worstConversion();
    }
  }

  if( bestViableFunction.isViable() )
    return dynamic_cast<Declaration*>(bestViableFunction.declaration().data());
  else
    return 0;
}

QList< ViableFunction > OverloadResolver::resolveListOffsetted( const ParameterList& params, const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, bool partial  )
{
  if( !m_context || !m_topContext )
    return QList<ViableFunction>();
  
  ///Iso c++ draft 13.3.3
  m_worstConversionRank = ExactMatch;

  ///First step: Replace class-instances with operator() functions, and pure classes with their constructors
  QList<QPair<OverloadResolver::ParameterList, Declaration*> > newDeclarations = declarations;
  expandDeclarations( declarations, newDeclarations );
  
  ///Second step: Find best viable function
  QList<ViableFunction> viableFunctions;

  for( QList<QPair<OverloadResolver::ParameterList, Declaration*> >::const_iterator it = newDeclarations.begin(); it != newDeclarations.end(); ++it )
  {
    ViableFunction viable( m_topContext.data(), (*it).second );
    ParameterList mergedParams = (*it).first;
    mergedParams.parameters += params.parameters;
    viable.matchParameters( mergedParams, partial );

    viableFunctions << viable;
  }

  qSort( viableFunctions );
  
  return viableFunctions;
}
