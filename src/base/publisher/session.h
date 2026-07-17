//
// Created by getroot on 18. 3. 19.
//

#pragma once

#include <base/ovlibrary/ovlibrary.h>

#include "base/common_types.h"
#include "base/info/session.h"

#include <vector>
#include <set>
#include <map>
#include <mutex>

#define OV_LOG_TAG "Session"

namespace pub
{
	class Application;
	class Stream;

	class Session : public info::Session
	{
	public:
		Session(const std::shared_ptr<Application> &application, const std::shared_ptr<Stream> &stream);
		Session(const info::Session &info, const std::shared_ptr<Application> &app, const std::shared_ptr<Stream> &stream);
		virtual ~Session();

		const std::shared_ptr<Application> &GetApplication();
		std::shared_ptr<const Application> GetApplication() const;
		const std::shared_ptr<Stream> &GetStream();
		std::shared_ptr<const Stream> GetStream() const;

		std::shared_ptr<ov::Url> GetRequestedUrl() const;
		void SetRequestedUrl(const std::shared_ptr<ov::Url> &requested_url);

		std::shared_ptr<ov::Url> GetFinalUrl() const;
		void SetFinalUrl(const std::shared_ptr<ov::Url> &final_url);

		virtual bool Start();
		virtual bool Stop();

		virtual void SendOutgoingData(const std::any &packet) {};
		virtual void OnMessageReceived(const std::any &message) {};

		virtual std::vector<ov::String> GetActiveConnectionIds() const { return {}; }

		ov::String GetViewerId() const { return GetViewerIds().empty() ? "" : *GetViewerIds().begin(); }
		void SetViewerId(const ov::String &viewer_id) { AddViewerId(0, viewer_id); }

		std::set<ov::String> GetViewerIds() const
		{
			std::lock_guard<std::mutex> lock(_viewer_ids_mutex);
			return _viewer_ids;
		}

		void AddViewerId(uint32_t connection_id, const ov::String &viewer_id)
		{
			std::lock_guard<std::mutex> lock(_viewer_ids_mutex);
			logti("viewer id %s", viewer_id.CStr());
			_viewer_ids.insert(viewer_id);
			_connection_viewer_ids[connection_id] = viewer_id;
		}

		bool HasConnection(uint32_t connection_id) const
		{
			std::lock_guard<std::mutex> lock(_viewer_ids_mutex);
			// logti("this instance: %p", this);			
			// logti("connection id %d", connection_id);
			bool dd = _connection_viewer_ids.find(connection_id) != _connection_viewer_ids.end();
			// logti("connection size %zu", _connection_viewer_ids.size());
			// for (const auto& pair : _connection_viewer_ids)
			// {
			// 	logti(" - Connection: %u -> Viewer ID: %s", pair.first, pair.second.CStr());
			// }
			// logti("has connection %d", dd);
			return dd;
		}

		void RemoveViewerIdForConnection(uint32_t connection_id)
		{
			std::lock_guard<std::mutex> lock(_viewer_ids_mutex);
			auto it = _connection_viewer_ids.find(connection_id);
			if (it != _connection_viewer_ids.end())
			{
				_viewer_ids.erase(it->second);
				_connection_viewer_ids.erase(it);
			}
		}

		enum class SessionState : int8_t
		{
			Ready,
			Started,
			Stopping,
			Stopped,
			Error
		};

		SessionState GetState();
		void SetState(SessionState state);
		virtual void Terminate(ov::String reason);

	protected:
		std::shared_ptr<ov::Url> _requested_url;
		std::shared_ptr<ov::Url> _final_url;

		std::set<ov::String> _viewer_ids;
		std::map<uint32_t, ov::String> _connection_viewer_ids;
		mutable std::mutex _viewer_ids_mutex;

	private:
		std::shared_ptr<Application> _application;
		std::shared_ptr<Stream> _stream;
		SessionState _state;
		ov::String _error_reason;
	};

}  // namespace pub