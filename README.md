# SendMessage
SendMessage is a little tool to send Windows messages to any window.

[![Build Status](https://tortoisesvn.visualstudio.com/tortoisesvnGitHub/_apis/build/status/stefankueng.sendmessage)](https://tortoisesvn.visualstudio.com/tortoisesvnGitHub/_build/latest?definitionId=9)

Ever wondered how you should test whether your application correctly
responds to certain system messages like WM_ENDSESSION or WM_POWERBROADCAST?
Of course you can test your application by actually triggering those messages,
but especially the WM_ENDSESSION message and its purpose makes it impossible
to attach a debugger to your application once Windows sends you that message.

With this tool, you can send that message and any other message you
like to your application window. And you can do that while you have
a debugger attached to your application!

Please visit the [homepage](https://tools.stefankueng.com/SendMessage.html) of SendMessage for more information.
