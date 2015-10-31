#include "Sampler.h"

#pragma region BASE_SAMPLER

std::vector<Sample, Eigen::aligned_allocator<Vector3R>> Sampler::
sample_Diffuse(const Vector3R &normal, int sampleSize) {
	Vector3R dummy(uniform_01(gen), uniform_01(gen), uniform_01(gen));
	Vector3R x = dummy.cross(normal).normalized();
	Vector3R y = normal.cross(x);
	std::vector<Sample2R> samples2R = sample_UnitSquare_Uniform(sampleSize);
	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples3R(sampleSize * sampleSize);

	for (int i = 0; i < samples2R.size(); i++) {
		Real sq = sqrt(samples2R[i].u);
		samples3R[i].v =
			((sq * cos(2 * Constants::Pi * samples2R[i].v)) * x) +
			((sq * sin(2 * Constants::Pi * samples2R[i].v)) * y) +
			sqrt(1 - samples2R[i].u) * normal;
		samples3R[i].weight = 1;
	}
	return samples3R;
}

std::vector<Sample, Eigen::aligned_allocator<Vector3R>> Sampler::
sample_Specular(const Vector3R& out, const Vector3R& normal, int N, int sampleSize) {
	Vector3R x = out.cross(normal).normalized();
	Vector3R y = out.cross(x);
	std::vector<Sample2R> samples2R = sample_UnitSquare_Uniform(sampleSize);
	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples3R(sampleSize * sampleSize);
	for (int i = 0; i < samples2R.size(); i++) {
		Real cosphi = pow(1 - samples2R[i].u, 1. / N);
		Real sinphi = sqrt(1 - cosphi * cosphi);
		Real theta = 2 * Constants::Pi * samples2R[i].v;
		samples3R[i].v =
			(sinphi * cos(theta) * x) +
			(sinphi * sin(theta) * y) +
			cosphi * out;
		samples3R[i].weight = 1;
	}
	return samples3R;
}

void Sampler::generatePreSample_Diffuse(int groupSize, int numGroups) {
	presamples_Diffuse.cursor = 0;
	presamples_Diffuse.numGroups = numGroups;
	presamples_Diffuse.elemPerGroup = sample_UnitSquare_Uniform(groupSize).size();

	for (int i = 0; i < numGroups; i++) {
		std::vector<Sample2R> samples2R = sample_UnitSquare_Uniform(groupSize);
		for (int j = 0; j < presamples_Diffuse.elemPerGroup; j++) {
			Real sq = sqrt(samples2R[j].u);
			presamples_Diffuse.presamples.push_back(
				PreSample(
				sq * cos(2 * Constants::Pi * samples2R[j].v),
				sq * sin(2 * Constants::Pi * samples2R[j].v),
				sqrt(1 - samples2R[j].u),
				1));
		}
	}
}

void Sampler::generatePreSample_Diffuse_Single(int numSamples) {
	// this code copy-paste the above function and is not efficient, however it cannot be the bottleneck.
	presamples_Diffuse_Single.cursor = 0;
	presamples_Diffuse_Single.numGroups = numSamples;
	presamples_Diffuse_Single.elemPerGroup = sample_UnitSquare_Uniform(1).size();

	for (int i = 0; i < numSamples; i++) {
		std::vector<Sample2R> samples2R = sample_UnitSquare_Uniform(1);
		for (int j = 0; j < presamples_Diffuse_Single.elemPerGroup; j++) {
			Real sq = sqrt(samples2R[j].u);
			presamples_Diffuse_Single.presamples.push_back(
				PreSample(
				sq * cos(2 * Constants::Pi * samples2R[j].v),
				sq * sin(2 * Constants::Pi * samples2R[j].v),
				sqrt(1 - samples2R[j].u),
				1));
		}
	}
}

void Sampler::generatePreSample_Specular(int groupSize, int numGroups, int N) {
	if (presamples_Specular.find(N) != presamples_Specular.end() || N < 0) {
		return;
	}

	PreSamplePool psp;
	psp.cursor = 0;
	psp.numGroups = numGroups;
	psp.elemPerGroup = sample_UnitSquare_Uniform(groupSize).size();

	for (int i = 0; i < numGroups; i++) {
		std::vector<Sample2R> samples2R = sample_UnitSquare_Uniform(groupSize);
		for (int j = 0; j < psp.elemPerGroup; j++) {
			Real cosphi;
			if (N >= 1000) {
				cosphi = 1;
			}
			else {
				cosphi = pow(1 - samples2R[j].u, 1. / N);
			}
			Real sinphi = sqrt(1 - cosphi * cosphi);
			Real theta = 2 * Constants::Pi * samples2R[j].v;
			psp.presamples.push_back(PreSample(
				sinphi * cos(theta),
				sinphi * sin(theta),
				cosphi,
				1));
		}
	}
	presamples_Specular.insert(std::pair<int, PreSamplePool>(N, psp));
}

void Sampler::generatePreSample_Specular_Single(int numSamples, int N) {
	if (presamples_Specular_Single.find(N) != presamples_Specular_Single.end() || N < 0) {
		return;
	}
	PreSamplePool psp;
	psp.cursor = 0;
	psp.numGroups = numSamples;
	psp.elemPerGroup = sample_UnitSquare_Uniform(1).size();
	for (int i = 0; i < numSamples; i++) {
		std::vector<Sample2R> samples2R = sample_UnitSquare_Uniform(1);
		for (int j = 0; j < psp.elemPerGroup; j++) {
			Real cosphi;
			if (N >= 1000) {
				cosphi = 1;
			}
			else {
				cosphi = pow(1 - samples2R[j].u, 1. / N);
			}
			Real sinphi = sqrt(1 - cosphi * cosphi);
			Real theta = 2 * Constants::Pi * samples2R[j].v;
			psp.presamples.push_back(PreSample(
				sinphi * cos(theta),
				sinphi * sin(theta),
				cosphi,
				1));
		}
	}
	presamples_Specular_Single.insert(std::pair<int, PreSamplePool>(N, psp));
}

std::vector<Sample, Eigen::aligned_allocator<Vector3R>> Sampler::
sample_Diffuse_P(const Vector3R &normal, int sampleSize) {
	Vector3R dummy(uniform_01(gen), uniform_01(gen), uniform_01(gen));
	Vector3R x = dummy.cross(normal).normalized();
	Vector3R y = normal.cross(x);
	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples3R(presamples_Diffuse.elemPerGroup);
	for (int i = 0; i < presamples_Diffuse.elemPerGroup; i++) {
		PreSample &ps = presamples_Diffuse.presamples[presamples_Diffuse.cursor + i];
		samples3R[i].v =
			(ps.x * x) +
			(ps.y * y) +
			ps.z * normal;
		samples3R[i].weight = ps.weight;
	}
	presamples_Diffuse.cursor = 
		(presamples_Diffuse.elemPerGroup + presamples_Diffuse.cursor) % (presamples_Diffuse.elemPerGroup * presamples_Diffuse.numGroups);
	return samples3R;
}

std::vector<Sample, Eigen::aligned_allocator<Vector3R>> Sampler::
sample_Specular_P(const Vector3R &out, const Vector3R &normal, int N, int sampleSize) {
	Vector3R x = out.cross(normal).normalized();
	Vector3R y = out.cross(x);
	PreSamplePool &psp = presamples_Specular[N];

	std::vector<Sample, Eigen::aligned_allocator<Vector3R>> samples3R(psp.elemPerGroup);
	for (int i = 0; i < psp.elemPerGroup; i++) {
		PreSample &ps = psp.presamples[presamples_Diffuse.cursor + i];
		samples3R[i].v =
			(ps.x * x) +
			(ps.y * y) +
			ps.z * out;
		samples3R[i].weight = ps.weight;
	}
	psp.cursor = (psp.elemPerGroup + psp.cursor) % (psp.elemPerGroup * psp.numGroups);
	return samples3R;
}

Vector3R Sampler::sample_Diffuse_P(const Vector3R &normal) {
	Vector3R dummy(uniform_01(gen), uniform_01(gen), uniform_01(gen));
	Vector3R x = dummy.cross(normal).normalized();
	Vector3R y = normal.cross(x);
	PreSample &ps = presamples_Diffuse_Single.presamples[presamples_Diffuse_Single.cursor];
	++presamples_Diffuse_Single.cursor %= presamples_Diffuse_Single.numGroups;
	return (ps.x) * x + ps.y * y + ps.z * normal;
}

Vector3R Sampler::sample_Specular_P(const Vector3R &out, const Vector3R &normal, int N) {
	Vector3R dummy(uniform_01(gen), uniform_01(gen), uniform_01(gen));
	Vector3R x = dummy.cross(normal).normalized();
	Vector3R y = normal.cross(x);
	PreSamplePool &psp = presamples_Specular_Single[N];
	PreSample &ps = psp.presamples[psp.cursor];
	++psp.cursor %= psp.numGroups;
	return (ps.x) * x + ps.y * y + ps.z * out;
}

Sample Sampler::sample(const Vector3R &in, const Vector3R &normal, const MaterialPtr &mPtr) {
	Sample ans;
	bool lookFromOutside = in.dot(normal) < 0;
	if (lookFromOutside && mPtr->Tr < 0) {
		// from outside, reflective diffuse and specular light
		Real wd = (mPtr->Kd[3]) / (mPtr->Kd[3] + mPtr->Ks[3]);
		if (uniform_01(gen) < wd) {
			ans.type = RE_D;
			ans.weight = wd;
			ans.v = sample_Diffuse_P(normal);
			return ans;
		}
		else {
			ans.type = RE_S;
			ans.weight = 1 - wd;
			ans.v = sample_Specular_P(IntersectionTester::reflect(in, normal), normal, mPtr->Ns);
			return ans;
		}
	}
	else if (lookFromOutside && mPtr->Tr > 0) {
		// look from outside, transparent material
		Real total = (mPtr->Kd[3] + mPtr->Ks[3] + mPtr->Td[3] + mPtr->Ts[3]);
		Real w, t = 0;
		Real rand = uniform_01(gen);
		if (rand < (t += (w = mPtr->Kd[3] / total))) {
			ans.type = RE_D;
			ans.weight = w;
			ans.v = sample_Diffuse_P(normal);
			return ans;
		}
		else if (rand < (t += (w = mPtr->Ks[3] / total))) {
			ans.type = RE_S;
			ans.weight = w;
			ans.v = sample_Specular_P(IntersectionTester::reflect(in, normal), normal, mPtr->Ns);
			return ans;
		}
		else if (rand < (t += (w = mPtr->Td[3] / total))) {
			ans.type = TR_D;
			ans.weight = w;
			ans.v = sample_Diffuse_P(-normal);
			return ans;
		}
		else {
			ans.type = TR_S;
			ans.weight = 1 - t;
			ans.v = sample_Specular_P(IntersectionTester::refract(in, normal, mPtr->n), normal, mPtr->Nst);
			return ans;
		}
	}
	else {
		// look from inside, transparent material
		// to do: fresnel
		Real wd = (mPtr->Td[3]) / (mPtr->Td[3] + mPtr->Ts[3]);
		if (uniform_01(gen) < wd) {
			ans.type = TR_D;
			ans.weight = wd;
			ans.v = sample_Diffuse_P(normal);
			return ans;
		}
		else {
			ans.type = TR_S;
			ans.weight = 1 - wd;
			ans.v = sample_Specular_P(IntersectionTester::refract(in, normal, mPtr->n), normal, mPtr->Ns);
			return ans;
		}
	}
}

#pragma endregion

#pragma region STRATIFIED_SAMPLER

std::vector<Sample2R> StratifiedSampler::sample_UnitSquare_Uniform(int sampleSize) {
	Real step = 1.0 / sampleSize;
	std::vector<Sample2R> samples2R(sampleSize * sampleSize);
	for (int i = 0; i < sampleSize; i++) {
		for (int j = 0; j < sampleSize; j++) {
			samples2R[i * sampleSize + j] = Sample2R((i + uniform_01(gen)) * step, (j + uniform_01(gen)) * step);
		}
	}
	return samples2R;
}

#pragma endregion

#pragma region LATINCUBE_SAMPLER

std::vector<Sample2R> LatinCubeSampler::sample_UnitSquare_Uniform(int sampleSize) {
	Real step = 1.0 / sampleSize;
	std::vector<Sample2R> samples2R(sampleSize);
	std::vector<int> permutation(sampleSize);
	for (int i = 0; i < sampleSize; i++) {
		samples2R[i] = Sample2R(uniform_01(gen), uniform_01(gen));
		permutation[i] = i;
	}
	std::shuffle(permutation.begin(), permutation.end(), std::default_random_engine());
	for (int i = 0; i < sampleSize; i++) {
		samples2R[i].u = (samples2R[i].u + permutation[i]) * step;
		samples2R[i].v = (i + samples2R[i].v) * step;
	}
	return samples2R;
}

#pragma endregion