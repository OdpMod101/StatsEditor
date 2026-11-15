#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <cstdio>
#include <cstdlib>

using namespace geode::prelude;

class StatsEditorPopup : public geode::Popup {
protected:
    TextInput* m_starsInput = nullptr;
    TextInput* m_moonsInput = nullptr;
    TextInput* m_diamondsInput = nullptr;
    TextInput* m_coinsInput = nullptr;
    TextInput* m_userCoinsInput = nullptr;
    TextInput* m_demonsInput = nullptr;
    ProfilePage* m_profilePage = nullptr;

    bool init(ProfilePage* page) {
        if (!Popup::init(320.f, 255.f)) return false;

        m_profilePage = page;
        this->setTitle("StatsEditor - Edit Account Stats");

        auto gsm = GameStatsManager::sharedState();
        auto winSize = m_mainLayer->getContentSize();

        auto createRow = [&](const char* labelText, const char* statKey, float yPos) -> TextInput* {
            auto label = CCLabelBMFont::create(labelText, "bigFont.fnt");
            label->setScale(0.45f);
            label->setAnchorPoint({0.f, 0.5f});
            label->setPosition({25.f, yPos});
            m_mainLayer->addChild(label);

            auto input = TextInput::create(120.f, "0", "bigFont.fnt");
            input->setCommonFilter(CommonFilter::Uint);
            input->setMaxCharCount(9);

            char buf[32];
            std::snprintf(buf, sizeof(buf), "%d", gsm->getStat(statKey));
            input->setString(buf);

            input->setPosition({winSize.width - 85.f, yPos});
            m_mainLayer->addChild(input);

            return input;
        };

        m_starsInput     = createRow("Stars:", "6", 205.f);
        m_moonsInput     = createRow("Moons:", "28", 175.f);
        m_diamondsInput  = createRow("Diamonds:", "13", 145.f);
        m_coinsInput     = createRow("Secret Coins:", "8", 115.f);
        m_userCoinsInput = createRow("User Coins:", "12", 85.f);
        m_demonsInput    = createRow("Demons:", "5", 55.f);

        auto saveSpr = ButtonSprite::create("Save & Sync", "goldFont.fnt", "GJ_button_01.png", 0.8f);
        auto saveBtn = CCMenuItemSpriteExtra::create(
            saveSpr,
            this,
            menu_selector(StatsEditorPopup::onSave)
        );
        saveBtn->setPosition({winSize.width / 2.f, 22.f});
        m_buttonMenu->addChild(saveBtn);

        return true;
    }

    void onSave(CCObject*) {
        auto gsm = GameStatsManager::sharedState();

        auto parseVal = [](TextInput* input, int fallback) -> int {
            auto str = input->getString();
            if (str.empty()) return fallback;
            char* endPtr = nullptr;
            long val = std::strtol(str.c_str(), &endPtr, 10);
            if (endPtr == str.c_str()) return fallback;
            return static_cast<int>(val);
        };

        gsm->setStat("6",  parseVal(m_starsInput, gsm->getStat("6")));
        gsm->setStat("28", parseVal(m_moonsInput, gsm->getStat("28")));
        gsm->setStat("13", parseVal(m_diamondsInput, gsm->getStat("13")));
        gsm->setStat("8",  parseVal(m_coinsInput, gsm->getStat("8")));
        gsm->setStat("12", parseVal(m_userCoinsInput, gsm->getStat("12")));
        gsm->setStat("5",  parseVal(m_demonsInput, gsm->getStat("5")));

        if (auto glm = GameLevelManager::sharedState()) {
            glm->updateUserScore();
        }

        if (m_profilePage && m_profilePage->m_score) {
            m_profilePage->loadPageFromUserInfo(m_profilePage->m_score);
        }

        this->onClose(nullptr);

        FLAlertLayer::create(
            "Stats Updated!",
            "Your stats have been <cg>saved</c> and synced to your online profile!",
            "OK"
        )->show();
    }

public:
    static StatsEditorPopup* create(ProfilePage* page) {
        auto ret = new StatsEditorPopup();
        if (ret->init(page)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(StatsEditorProfile, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        if (!m_ownProfile) return;
        if (this->getChildByIDRecursive("stats-editor-btn"_spr)) return;

        auto menu = this->getChildByIDRecursive("left-menu");
        if (!menu) return;

        auto spr = CircleButtonSprite::createWithSpriteFrameName(
            "GJ_starsIcon_001.png", 1.0f, CircleBaseColor::Green, CircleBaseSize::Small
        );

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(StatsEditorProfile::onOpenEditor)
        );
        btn->setID("stats-editor-btn"_spr);

        menu->addChild(btn);
        menu->updateLayout();
    }

    void onOpenEditor(CCObject*) {
        StatsEditorPopup::create(this)->show();
    }
};
