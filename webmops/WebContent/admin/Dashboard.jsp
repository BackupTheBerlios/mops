<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="file list">
	<stripes:layout-component name="contents">
		<table class="dashboard" cellpadding="2" cellspacing="0" border="0" width="100%">
			<tr class="dashboard">
				<td class="dashboard" width="40%"> <!--  users -->
					<div id="dashboard-pane">
						<h2 class="dashboard-header">
							Users
						</h2>
						<br />
						<c:choose>
							<c:when test="${actionBean.usersCount == 0 }">
								<strong>No users</strong>
							</c:when>
							<c:when test="${actionBean.usersCount > 0 }">
								<table class="commons-list" width="100%">
									<c:forEach items="${actionBean.users}" var="user" varStatus="rowstat">
										<tr class="common-list ${rowstat.count mod 2 == 0 ? 'even' : 'odd' }">
											<td>
												<stripes:link beanclass="ru.rpunet.webmops.web.admin.AddEditUserActionBean">
													<stripes:param name="personId" value="${user.id}" />
													${user.login}
												</stripes:link>
											</td>
											<td>${user.group}</td>
											<td>${user.active}</td>
										</tr>
									</c:forEach>	
								</table>
							</c:when>
						</c:choose>
					</div>
				</td> <!--  /users -->
				<td width="10%">&nbsp;</td>
				<td class="dashboard" width="40%"> <!-- mirrors -->
					<div id="dashboard-pane">
						<h2 class="dashboard-header">
							Mirrors
						</h2>
						<br />
						<c:choose>
							<c:when test="${actionBean.mirrorsCount == 0 }">
								<strong>No mirrors</strong>
							</c:when>
							<c:when test="${actionBean.mirrorsCount > 0 }">
								<table class="common-list" width="100%">
									<c:forEach items="${actionBean.mirrors}" var="mirror" varStatus="rowstat">
										<tr class="common-list ${rowstat.count mod 2 == 0 ? 'even' : 'odd' }">
											<td>${mirror.name}</td>
											<td>${mirror.url}</td>
											<td>${mirror.type}</td>
										</tr>
									</c:forEach>	
								</table>
							</c:when>
						</c:choose>
					</div>
				</td> <!-- /mirrors -->
			</tr>
			
			<tr>
				<td>
					<div id="dashboard-pane">
						<h2 class="dashboard-header">
							Distributions
						</h2>
						<br />
						<c:choose>
							<c:when test="${actionBean.distsCount == 0 }">
								<strong>No distributiions</strong>
							</c:when>
							<c:when test="${actionBean.distsCount > 0 }">
								<table class="common-list" width="100%">
									<c:forEach items="${actionBean.dists}" var="dist" varStatus="rowstat">
										<tr class="common-list ${rowstat.count mod 2 == 0 ? 'even' : 'odd' }">
											<td>${dist.name}</td>
											<td>${dist.description}</td>
										</tr>
									</c:forEach>	
								</table>
							</c:when>
						</c:choose>
					</div>
				</td>
				<td colspan="2">&nbsp;
				</td>
			</tr>
		</table>
	</stripes:layout-component>
</stripes:layout-render>