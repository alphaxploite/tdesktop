/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/required.h"
#include "api/api_common.h"
#include "base/unique_qptr.h"
#include "history/view/controls/compose_controls_common.h"
#include "ui/rp_widget.h"
#include "ui/effects/animations.h"
#include "ui/widgets/input_fields.h"
#include "chat_helpers/tabbed_selector.h"

class History;
class FieldAutocomplete;

namespace ChatHelpers {
class TabbedPanel;
class TabbedSelector;
} // namespace ChatHelpers

namespace Data {
struct MessagePosition;
} // namespace Data

namespace InlineBots {
namespace Layout {
class ItemBase;
class Widget;
} // namespace Layout
class Result;
} // namespace InlineBots

namespace Ui {
class SendButton;
class IconButton;
class EmojiButton;
} // namespace Ui

namespace Main {
class Session;
} // namespace Main

namespace Window {
class SessionController;
struct SectionShow;
} // namespace Window

namespace Api {
enum class SendProgressType;
} // namespace Api

namespace HistoryView {

namespace Controls {
class VoiceRecordBar;
} // namespace Controls

class FieldHeader;

class ComposeControls final {
public:
	using FileChosen = ChatHelpers::TabbedSelector::FileChosen;
	using PhotoChosen = ChatHelpers::TabbedSelector::PhotoChosen;
	using InlineChosen = ChatHelpers::TabbedSelector::InlineChosen;

	using MessageToEdit = Controls::MessageToEdit;
	using VoiceToSend = Controls::VoiceToSend;
	using SendActionUpdate = Controls::SendActionUpdate;
	using SetHistoryArgs = Controls::SetHistoryArgs;

	enum class Mode {
		Normal,
		Scheduled,
	};

	ComposeControls(
		not_null<Ui::RpWidget*> parent,
		not_null<Window::SessionController*> window,
		Mode mode);
	~ComposeControls();

	[[nodiscard]] Main::Session &session() const;
	void setHistory(SetHistoryArgs &&args);
	void finishAnimating();

	void move(int x, int y);
	void resizeToWidth(int width);
	void setAutocompleteBoundingRect(QRect rect);
	[[nodiscard]] rpl::producer<int> height() const;
	[[nodiscard]] int heightCurrent() const;

	bool focus();
	[[nodiscard]] rpl::producer<> cancelRequests() const;
	[[nodiscard]] rpl::producer<> sendRequests() const;
	[[nodiscard]] rpl::producer<VoiceToSend> sendVoiceRequests() const;
	[[nodiscard]] rpl::producer<QString> sendCommandRequests() const;
	[[nodiscard]] rpl::producer<MessageToEdit> editRequests() const;
	[[nodiscard]] rpl::producer<> attachRequests() const;
	[[nodiscard]] rpl::producer<FileChosen> fileChosen() const;
	[[nodiscard]] rpl::producer<PhotoChosen> photoChosen() const;
	[[nodiscard]] rpl::producer<Data::MessagePosition> scrollRequests() const;
	[[nodiscard]] rpl::producer<not_null<QKeyEvent*>> keyEvents() const;
	[[nodiscard]] rpl::producer<InlineChosen> inlineResultChosen() const;
	[[nodiscard]] rpl::producer<SendActionUpdate> sendActionUpdates() const;

	using MimeDataHook = Fn<bool(
		not_null<const QMimeData*> data,
		Ui::InputField::MimeAction action)>;
	void setMimeDataHook(MimeDataHook hook);

	bool pushTabbedSelectorToThirdSection(
		not_null<PeerData*> peer,
		const Window::SectionShow &params);
	bool returnTabbedSelector();

	[[nodiscard]] bool isEditingMessage() const;
	[[nodiscard]] FullMsgId replyingToMessage() const;

	void showForGrab();
	void showStarted();
	void showFinished();
	void raisePanels();

	void editMessage(FullMsgId id);
	void cancelEditMessage();

	void replyToMessage(FullMsgId id);
	void cancelReplyMessage();

	bool handleCancelRequest();

	[[nodiscard]] TextWithTags getTextWithAppliedMarkdown() const;
	[[nodiscard]] WebPageId webPageId() const;
	void setText(const TextWithTags &text);
	void clear();
	void hidePanelsAnimated();

	[[nodiscard]] rpl::producer<bool> lockShowStarts() const;
	[[nodiscard]] bool isLockPresent() const;
	[[nodiscard]] bool isRecording() const;

private:
	enum class TextUpdateEvent {
		//SaveDraft = (1 << 0),
		SendTyping = (1 << 1),
	};
	using TextUpdateEvents = base::flags<TextUpdateEvent>;
	friend inline constexpr bool is_flag_type(TextUpdateEvent) { return true; };

	void init();
	void initField();
	void initTabbedSelector();
	void initSendButton();
	void initWebpageProcess();
	void initWriteRestriction();
	void initVoiceRecordBar();
	void initAutocomplete();
	void updateSubmitSettings();
	void updateSendButtonType();
	void updateHeight();
	void updateWrappingVisibility();
	void updateControlsVisibility();
	void updateControlsGeometry(QSize size);
	void updateOuterGeometry(QRect rect);
	void paintBackground(QRect clip);

	void orderControls();
	void checkAutocomplete();
	void updateStickersByEmoji();
	void updateFieldPlaceholder();

	void escape();
	void fieldChanged();
	void fieldTabbed();
	void toggleTabbedSelectorMode();
	void createTabbedPanel();
	void setTabbedPanel(std::unique_ptr<ChatHelpers::TabbedPanel> panel);

	void setTextFromEditingMessage(not_null<HistoryItem*> item);

	bool showRecordButton() const;
	void drawRestrictedWrite(Painter &p, const QString &error);
	void updateOverStates(QPoint pos);

	void cancelInlineBot();
	void clearInlineBot();
	void inlineBotChanged();

	// Look in the _field for the inline bot and query string.
	void updateInlineBotQuery();

	// Request to show results in the emoji panel.
	void applyInlineBotQuery(UserData *bot, const QString &query);

	void inlineBotResolveDone(const MTPcontacts_ResolvedPeer &result);
	void inlineBotResolveFail(const RPCError &error, const QString &username);

	const not_null<QWidget*> _parent;
	const not_null<Window::SessionController*> _window;
	History *_history = nullptr;
	Fn<bool()> _showSlowmodeError;
	rpl::variable<int> _slowmodeSecondsLeft;
	rpl::variable<bool> _sendDisabledBySlowmode;
	rpl::variable<std::optional<QString>> _writeRestriction;
	Mode _mode = Mode::Normal;

	const std::unique_ptr<Ui::RpWidget> _wrap;
	const std::unique_ptr<Ui::RpWidget> _writeRestricted;

	const std::shared_ptr<Ui::SendButton> _send;
	const not_null<Ui::IconButton*> _attachToggle;
	const not_null<Ui::EmojiButton*> _tabbedSelectorToggle;
	const not_null<Ui::InputField*> _field;
	std::unique_ptr<InlineBots::Layout::Widget> _inlineResults;
	std::unique_ptr<ChatHelpers::TabbedPanel> _tabbedPanel;
	std::unique_ptr<FieldAutocomplete> _autocomplete;

	friend class FieldHeader;
	const std::unique_ptr<FieldHeader> _header;
	const std::unique_ptr<Controls::VoiceRecordBar> _voiceRecordBar;

	rpl::event_stream<> _cancelRequests;
	rpl::event_stream<FileChosen> _fileChosen;
	rpl::event_stream<PhotoChosen> _photoChosen;
	rpl::event_stream<InlineChosen> _inlineResultChosen;
	rpl::event_stream<SendActionUpdate> _sendActionUpdates;
	rpl::event_stream<QString> _sendCommandRequests;

	TextWithTags _localSavedText;
	TextUpdateEvents _textUpdateEvents;

	//bool _inReplyEditForward = false;
	//bool _inClickable = false;

	UserData *_inlineBot = nullptr;
	QString _inlineBotUsername;
	bool _inlineLookingUpBot = false;
	mtpRequestId _inlineBotResolveRequestId = 0;
	bool _isInlineBot = false;

	rpl::lifetime _uploaderSubscriptions;

	Fn<void()> _raiseEmojiSuggestions;

};

} // namespace HistoryView