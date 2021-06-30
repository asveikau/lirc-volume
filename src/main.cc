/*
 Copyright (C) 2021 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <stdio.h>
#include <string.h>
#include <vector>

#include <common/error.h>
#include <common/logger.h>
#include <common/misc.h>

#include <common/c++/refcount.h>

#include <pollster/sockapi.h>
#include <pollster/pollster.h>

#include <AudioDevice.h>

#define LIRC_SOCKET "/var/run/lirc/lircd"
#define VOLUME_INC  0.05f

struct State
{
   common::Pointer<audio::Mixer> mixer;
   std::vector<char> pendingSocketData;

   void
   ChangeVolume(float (*filter)(float), error *err)
   {
      auto idx = 0;
      auto channels = mixer->GetChannels(idx, err);
      if (ERROR_FAILED(err))
         return;

      float data[channels]; // XXX
      channels = mixer->GetValue(idx, data, channels, err);
      ERROR_CHECK(err);

      for (int i=0; i<channels; ++i)
      {
         auto change = filter(data[i]);
         data[i] = MAX(0.0f, MIN(change, 1.0f));
      }

      mixer->SetValue(idx, data, channels, err);
      ERROR_CHECK(err);
   exit:;
   }

   void
   ProcessCommand(char *buf, error *err)
   {
      // Format is: ID<SPACE>REPEAT<SPACE>KEY<SPACE>REMOTE.
      // We shall remove all but the key.
      //
      buf = strchr(buf, ' ');
      if (!buf) return;
      buf = strchr(buf + 1, ' ');
      if (!buf) return;
      ++buf;
      char *q = strchr(buf, ' ');
      if (q) *q = 0;

      const char *key = buf;
      if (!strcmp(key, "KEY_VOLUMEUP"))
      {
         ChangeVolume([] (float f) -> float { return f + VOLUME_INC; }, err);

         error dummy;
         if (!ERROR_FAILED(err) && mixer->IsMuted(0, &dummy))
            mixer->SetMute(0, false, err);
      }
      else if (!strcmp(key, "KEY_VOLUMEDOWN"))
      {
         ChangeVolume([] (float f) -> float { return f - VOLUME_INC; }, err);
      }
      else if (!strcmp(key, "KEY_MUTE"))
      {
         error dummy;
         bool muted = mixer->IsMuted(0, &dummy);
         mixer->SetMute(0, !muted, err);
      }
   }

   void
   Append(const void *buf, size_t len, error *err)
   {
      try
      {
         pendingSocketData.insert(pendingSocketData.end(), (const char*)buf, (const char*)buf + len);
      }
      catch (const std::bad_alloc &)
      {
         error_set_nomem(err);
      }
   }

   char *
   FindNewline(char *buf, size_t len)
   {
      while (len && *buf != '\n')
      {
         ++buf;
         --len;
      }
      return len && *buf == '\n' ? buf : nullptr;
   }

   char *
   ProcessCommands(char *buf, size_t len, error *err)
   {
      char *p;
      while ((p = FindNewline(buf, len)))
      {
         *p++ = 0;
         ProcessCommand(buf, err);
         ERROR_CHECK(err);
         len -= (p - buf);
         buf = p;
      }
   exit:
      return buf;
   }

   void
   OnSocketData(const void *buf, size_t len, error *err)
   {
      if (pendingSocketData.size())
      {
         Append(buf, len, err);
         ERROR_CHECK(err);

         char *end = ProcessCommands(pendingSocketData.data(), pendingSocketData.size(), err);
         ERROR_CHECK(err);
         if (end != pendingSocketData.data())
         {
            auto delta = end - pendingSocketData.data();
            pendingSocketData.erase(pendingSocketData.begin(), pendingSocketData.begin()+delta);
         }
      }
      else
      {
         char *buf_s = (char*)buf;
         auto next = ProcessCommands(buf_s, len, err);
         ERROR_CHECK(err);
         auto newLen = len - (next - buf_s);
         if (newLen)
         {
            Append(next, newLen, err);
            ERROR_CHECK(err);
         }
      }

   exit:;
   }
};

int
main(int argc, char **argv)
{
   error err;
   common::Pointer<pollster::waiter> eventLoop;
   common::Pointer<audio::DeviceEnumerator> devEnum;
   auto sock = std::make_shared<pollster::StreamSocket>();
   auto state = std::make_shared<State>();

   bool gotStop = false;

   log_register_callback(
      [] (void *np, const char *p) -> void { fputs(p, stderr); },
      nullptr
   );

   audio::GetDeviceEnumerator(devEnum.GetAddressOf(), &err);
   ERROR_CHECK(&err);

   devEnum->GetDefaultMixer(state->mixer.GetAddressOf(), &err);
   ERROR_CHECK(&err);

   pollster::create(eventLoop.GetAddressOf(), &err);
   ERROR_CHECK(&err);

   pollster::set_common_queue(eventLoop.Get());

   sock->on_closed = sock->on_error = [&gotStop] (error *err) -> void { gotStop = true; };
   sock->on_recv = [state] (const void *buf, size_t len, error *err) -> void
   {
      state->OnSocketData(buf, len, err);
   };

   sock->ConnectUnixDomain(LIRC_SOCKET);

   while (!gotStop)
   {
      eventLoop->exec(&err);
      ERROR_CHECK(&err);
   }

   return 0;
exit:
   return 1;
}
