#include "Encyclopedia.h"

#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../parse/Parse.h"

const Encyclopedia& GetEncyclopedia() {
    static Encyclopedia encyclopedia;
    return encyclopedia;
}

Encyclopedia::Encyclopedia() :
    articles(),
    empty_article()
{
    try {
        parse::encyclopedia_articles(*this);
    } catch (const std::exception& e) {
        ErrorLogger() << "Failed parsing encyclopedia articles: error: " << e.what();
        throw e;
    }

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        DebugLogger() << "(Category) Encyclopedia Articles:";
        for (std::map<std::string, std::vector<EncyclopediaArticle> >::value_type& entry : articles) {
            const std::string& category = entry.first;
            for (const EncyclopediaArticle& article : entry.second)
            { DebugLogger() << "(" << category << ") : " << article.name; }
        }
    }
}
