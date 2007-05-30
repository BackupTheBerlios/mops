/* Copyright 2007 Andrew Diakin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ru.rpunet.webmops.dao;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.hibernate.Session;

import ru.rpunet.webmops.model.Mirror;
import ru.rpunet.webmops.utils.HibernateUtil;

/**
 * @author Andrew Diakin
 *
 */
public class MirrorManager {
	private Map<Long, Mirror> mirrors;
	private Session session;
	
	@SuppressWarnings("unchecked")
	public MirrorManager() {
		mirrors = new HashMap<Long, Mirror>();
		
		session = HibernateUtil.getSessionFactory().getCurrentSession();
		List<Mirror> m = (List<Mirror>) session.createCriteria(Mirror.class).list();
		for (Mirror mirror : m) {
			saveOrUpdate(mirror);
		}
	}
	
	public List<Mirror> findAll() {
		return Collections.unmodifiableList(new ArrayList<Mirror>(mirrors.values()));
	}
	
	public Mirror findMirrorById(Long id) {
		return mirrors.get(id);
	}
	
	public void makePersistent(Mirror mirror) {
		session.save(mirror);
	}
	
	public void saveOrUpdate(Mirror mirror) {
		mirrors.put(mirror.getId(), mirror);
	}
}
