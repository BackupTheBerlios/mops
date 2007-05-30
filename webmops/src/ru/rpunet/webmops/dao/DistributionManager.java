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

import ru.rpunet.webmops.model.Distribution;
import ru.rpunet.webmops.utils.HibernateUtil;

public class DistributionManager {
	private Map<Long, Distribution> dists;
	private Session session;
	
	
	@SuppressWarnings("unchecked")
	public DistributionManager() {
		
		dists = new HashMap<Long, Distribution>();
		
		session = HibernateUtil.getSessionFactory().getCurrentSession();
		List<Distribution> d = (List<Distribution>) session.createCriteria(Distribution.class).list();
		for (Distribution dist : d) {
			saveOrUpdate(dist);
		}
	}
	
	public List<Distribution> findAll() {
		return Collections.unmodifiableList(new ArrayList<Distribution>(dists.values()));
	}
	
	public Distribution findById(Long id) {
		return dists.get(id);
	}
	
	public void saveOrUpdate(Distribution dist) {
		dists.put(dist.getId(), dist);
	}
	
	public void makePersistent(Distribution dist) {
		session.save(dist);
	}
}
