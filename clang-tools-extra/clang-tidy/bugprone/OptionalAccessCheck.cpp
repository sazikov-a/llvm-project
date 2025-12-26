#include "OptionalAccessCheck.h"
#include "../utils/OptionsUtils.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::bugprone {

OptionalAccessCheck::OptionalAccessCheck(StringRef CheckName,
                                         ClangTidyContext *Context)
    : ClangTidyCheck(CheckName, Context),
      OptionalTypes(utils::options::parseStringList(
          Options.get("OptionalTypes",
                      "::std::optional;::absl::optional;::boost::optional"))) {}

void OptionalAccessCheck::registerMatchers(ast_matchers::MatchFinder *Finder) {

  auto BindOptionalType =
      qualType(hasDeclaration(cxxRecordDecl(hasAnyName(OptionalTypes))))
          .bind("optional-type");

  auto OptionalMatchType = hasType(hasCanonicalType(BindOptionalType));

  auto OptionalStarMatcher =
      cxxOperatorCallExpr(hasOverloadedOperatorName("*"),
                          hasUnaryOperand(OptionalMatchType))
          .bind("star-call");

  auto OptionalArrowMatcher =
      cxxOperatorCallExpr(hasOverloadedOperatorName("->"),
                          hasUnaryOperand(OptionalMatchType))
          .bind("arrow-call");

  Finder->addMatcher(callExpr(ignoringImpCasts(
                         anyOf(OptionalStarMatcher, OptionalArrowMatcher))),
                     this);
}

void OptionalAccessCheck::check(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  const auto *OptionalType = Result.Nodes.getNodeAs<QualType>("optional-type");
  const auto *OptionalStarCall =
      Result.Nodes.getNodeAs<CXXOperatorCallExpr>("star-call");
  const auto *OptionalArrowCall =
      Result.Nodes.getNodeAs<CXXOperatorCallExpr>("arrow-call");

  if (OptionalStarCall) {
    diag(OptionalStarCall->getExprLoc(),
         "remove error-prone optional access (via operator '*') to %0")
        << OptionalType->getUnqualifiedType();
    return;
  }

  if (OptionalArrowCall) {
    diag(OptionalArrowCall->getExprLoc(),
         "remove error-prone optional access (via operator '->') to %0")
        << OptionalType->getUnqualifiedType();
    return;
  }
}

void OptionalAccessCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "OptionalTypes",
                utils::options::serializeStringList(OptionalTypes));
}

std::optional<TraversalKind>
OptionalAccessCheck::getCheckTraversalKind() const {
  return TK_AsIs;
}

} // namespace clang::tidy::bugprone