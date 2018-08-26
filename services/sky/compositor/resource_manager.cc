// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/sky/compositor/resource_manager.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2extmojo.h>

#include "base/logging.h"
#include "base/stl_util.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/gpu/gl_context.h"
#include "mojo/gpu/gl_texture.h"
#include "services/sky/compositor/texture_layer.h"

namespace sky {

ResourceManager::ResourceManager(base::WeakPtr<mojo::GLContext> gl_context)
    : gl_context_(gl_context), next_resource_id_(0) {
}

ResourceManager::~ResourceManager() {
  STLDeleteContainerPairSecondPointers(resource_to_texture_map_.begin(),
                                       resource_to_texture_map_.end());
}

scoped_ptr<mojo::GLTexture> ResourceManager::CreateTexture(
    const gfx::Size& size) {
  scoped_ptr<mojo::GLTexture> texture = texture_cache_.GetTexture(size);
  if (texture)
    return texture.Pass();
  gl_context_->MakeCurrent();
  return make_scoped_ptr(new mojo::GLTexture(
      gl_context_, mojo::TypeConverter<mojo::Size, gfx::Size>::Convert(size)));
}

mojo::TransferableResourcePtr ResourceManager::CreateTransferableResource(
    TextureLayer* layer) {
  scoped_ptr<mojo::GLTexture> texture = layer->GetTexture();
  mojo::Size size = texture->size();

  gl_context_->MakeCurrent();
  glBindTexture(GL_TEXTURE_2D, texture->texture_id());
  GLbyte mailbox[GL_MAILBOX_SIZE_CHROMIUM];
  glGenMailboxCHROMIUM(mailbox);
  glProduceTextureCHROMIUM(GL_TEXTURE_2D, mailbox);
  GLuint sync_point = glInsertSyncPointCHROMIUM();
  glFlush();

  mojo::TransferableResourcePtr resource = mojo::TransferableResource::New();
  resource->id = next_resource_id_++;
  resource_to_texture_map_[resource->id] = texture.release();
  resource->format = mojo::ResourceFormat::RGBA_8888;
  resource->filter = GL_LINEAR;
  resource->size = size.Clone();
  resource->is_repeated = false;
  resource->is_software = false;

  mojo::MailboxHolderPtr mailbox_holder = mojo::MailboxHolder::New();
  mailbox_holder->mailbox = mojo::Mailbox::New();
  for (int i = 0; i < GL_MAILBOX_SIZE_CHROMIUM; ++i)
    mailbox_holder->mailbox->name.push_back(mailbox[i]);
  mailbox_holder->texture_target = GL_TEXTURE_2D;
  mailbox_holder->sync_point = sync_point;
  resource->mailbox_holder = mailbox_holder.Pass();

  return resource.Pass();
}

void ResourceManager::ReturnResources(
    mojo::Array<mojo::ReturnedResourcePtr> resources) {
  DCHECK(resources.size());

  gl_context_->MakeCurrent();
  for (size_t i = 0u; i < resources.size(); ++i) {
    mojo::ReturnedResourcePtr resource = resources[i].Pass();
    DCHECK_EQ(1, resource->count);
    auto iter = resource_to_texture_map_.find(resource->id);
    if (iter == resource_to_texture_map_.end())
      continue;
    scoped_ptr<mojo::GLTexture> texture(iter->second);
    DCHECK_NE(0u, texture->texture_id());
    resource_to_texture_map_.erase(iter);
    glWaitSyncPointCHROMIUM(resource->sync_point);
    texture_cache_.PutTexture(texture.Pass());
  }
}

}  // namespace examples
