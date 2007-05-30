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
   
package ru.rpunet.webmops.web.admin;

import java.util.List;

import ru.rpunet.webmops.model.Distribution;
import ru.rpunet.webmops.utils.WebmopsActionBean;
import ru.rpunet.webmops.dao.DistributionManager;

import net.sourceforge.stripes.action.DefaultHandler;
import net.sourceforge.stripes.action.ForwardResolution;
import net.sourceforge.stripes.action.Resolution;

public class DistributionsActionBean extends WebmopsActionBean {


	private List<Distribution> distributions;
	
	public int getDistCount() {
		return this.distributions.size();
	} 
	
	public List<Distribution> getDistributions() {
		return distributions;
	}

	@SuppressWarnings("unchecked")
	@DefaultHandler
	public Resolution list() {
		DistributionManager distDAO = new DistributionManager();
		distributions = distDAO.findAll();
		
		return new ForwardResolution("/admin/ListDistributions.jsp");
	}

}
